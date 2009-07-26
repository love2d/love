--
-- LANES.LUA
--
-- Multithreading and -core support for Lua
--
-- Author: Asko Kauppi <akauppi@gmail.com>
--
-- History:
--    Jun-08 AKa: major revise
--    15-May-07 AKa: pthread_join():less version, some speedup & ability to
--                   handle more threads (~ 8000-9000, up from ~ 5000)
--    26-Feb-07 AKa: serialization working (C side)
--    17-Sep-06 AKa: started the module (serialization)
--
--[[
===============================================================================

Copyright (C) 2007-08 Asko Kauppi <akauppi@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
]]--

module( "lanes", package.seeall )

require "lua51-lanes"
assert( type(lanes)=="table" )

local mm= lanes

local linda_id=    assert( mm.linda_id )

local thread_new=   assert(mm.thread_new)
local thread_status= assert(mm.thread_status)
local thread_join=  assert(mm.thread_join)
local thread_cancel= assert(mm.thread_cancel)

local _single= assert(mm._single)
local _version= assert(mm._version)

local _deep_userdata= assert(mm._deep_userdata)

local now_secs= assert( mm.now_secs )
local wakeup_conv= assert( mm.wakeup_conv )
local timer_gateway= assert( mm.timer_gateway )

local max_prio= assert( mm.max_prio )

-- This check is for sublanes requiring Lanes
--
-- TBD: We could also have the C level expose 'string.gmatch' for us. But this is simpler.
--
if not string then
    error( "To use 'lanes', you will also need to have 'string' available.", 2 )
end

-- 
-- Cache globals for code that might run under sandboxing 
--
local assert= assert
local string_gmatch= assert( string.gmatch )
local select= assert( select )
local type= assert( type )
local pairs= assert( pairs )
local tostring= assert( tostring )
local error= assert( error )
local setmetatable= assert( setmetatable )
local rawget= assert( rawget )

ABOUT= 
{
    author= "Asko Kauppi <akauppi@gmail.com>",
    description= "Running multiple Lua states in parallel",
    license= "MIT/X11",
    copyright= "Copyright (c) 2007-08, Asko Kauppi",
    version= _version,
}


-- Making copies of necessary system libs will pass them on as upvalues;
-- only the first state doing "require 'lanes'" will need to have 'string'
-- and 'table' visible.
--
local function WR(str)
    io.stderr:write( str.."\n" )
end

local function DUMP( tbl )
    if not tbl then return end
    local str=""
    for k,v in pairs(tbl) do
        str= str..k.."="..tostring(v).."\n"
    end
    WR(str)
end


---=== Laning ===---

-- lane_h[1..n]: lane results, same as via 'lane_h:join()'
-- lane_h[0]:    can be read to make sure a thread has finished (always gives 'true')
-- lane_h[-1]:   error message, without propagating the error
--
--      Reading a Lane result (or [0]) propagates a possible error in the lane
--      (and execution does not return). Cancelled lanes give 'nil' values.
--
-- lane_h.state: "pending"/"running"/"waiting"/"done"/"error"/"cancelled"
--
local lane_mt= {
    __index= function( me, k )
                if type(k) == "number" then
                    -- 'me[0]=true' marks we've already taken in the results
                    --
                    if not rawget( me, 0 ) then
                        -- Wait indefinately; either propagates an error or
                        -- returns the return values
                        --
                        me[0]= true  -- marker, even on errors

                        local t= { thread_join(me._ud) }   -- wait indefinate
                            --
                            -- { ... }      "done": regular return, 0..N results
                            -- { }          "cancelled"
                            -- { nil, err_str, stack_tbl } "error"
                        
                        local st= thread_status(me._ud)
                        if st=="done" then
                            -- Use 'pairs' and not 'ipairs' so that nil holes in
                            -- the returned values are tolerated.
                            --
                            for i,v in pairs(t) do
                                me[i]= v
                            end
                        elseif st=="error" then
                            assert( t[1]==nil and t[2] and type(t[3])=="table" )
                            me[-1]= t[2]
                            -- me[-2] could carry the stack table, but even 
                            -- me[-1] is rather unnecessary (and undocumented);
                            -- use ':join()' instead.   --AKa 22-Jan-2009
                        elseif st=="cancelled" then
                            -- do nothing
                        else
                            error( "Unexpected status: "..st )
                        end
                    end

                    -- Check errors even if we'd first peeked them via [-1]
                    -- and then came for the actual results.
                    --
                    local err= rawget(me, -1)
                    if err~=nil and k~=-1 then
                        -- Note: Lua 5.1 interpreter is not prepared to show
                        --       non-string errors, so we use 'tostring()' here
                        --       to get meaningful output.  --AKa 22-Jan-2009
                        --
                        --       Also, the stack dump we get is no good; it only
                        --       lists our internal Lanes functions. There seems
                        --       to be no way to switch it off, though.
                        
                        -- Level 3 should show the line where 'h[x]' was read
                        -- but this only seems to work for string messages
                        -- (Lua 5.1.4). No idea, why.   --AKa 22-Jan-2009
                        --
                        error( tostring(err), 3 )   -- level 3 should show the line where 'h[x]' was read
                    end
                    return rawget( me, k )
                    --
                elseif k=="status" then     -- me.status
                    return thread_status(me._ud)
                    --
                else
                    error( "Unknown key: "..k )
                end
             end
    }

-----
-- h= lanes.gen( [libs_str|opt_tbl [, ...],] lane_func ) ( [...] )
--
-- 'libs': nil:     no libraries available (default)
--         "":      only base library ('assert', 'print', 'unpack' etc.)
--         "math,os": math + os + base libraries (named ones + base)
--         "*":     all standard libraries available
--
-- 'opt': .priority:  int (-2..+2) smaller is lower priority (0 = default)
--
--	      .cancelstep: bool | uint
--            false: cancellation check only at pending Linda operations
--                   (send/receive) so no runtime performance penalty (default)
--            true:  adequate cancellation check (same as 100)
--            >0:    cancellation check every x Lua lines (small number= faster
--                   reaction but more performance overhead)
--
--        .globals:  table of globals to set for a new thread (passed by value)
--
--        ... (more options may be introduced later) ...
--
-- Calling with a function parameter ('lane_func') ends the string/table
-- modifiers, and prepares a lane generator. One can either finish here,
-- and call the generator later (maybe multiple times, with different parameters) 
-- or add on actual thread arguments to also ignite the thread on the same call.
--
local lane_proxy

local valid_libs= {
    ["package"]= true,
    ["table"]= true,
    ["io"]= true,
    ["os"]= true,
    ["string"]= true,
    ["math"]= true,
    ["debug"]= true,
    --
    ["base"]= true,
    ["coroutine"]= true,
    ["*"]= true
}

function gen( ... )
    local opt= {}
    local libs= nil
    local lev= 2  -- level for errors

    local n= select('#',...)
    
    if n==0 then
        error( "No parameters!" )
    end

    for i=1,n-1 do
        local v= select(i,...)
        if type(v)=="string" then
            libs= libs and libs..","..v or v
        elseif type(v)=="table" then
            for k,vv in pairs(v) do
                opt[k]= vv
            end
        elseif v==nil then
            -- skip
        else
            error( "Bad parameter: "..tostring(v) )
        end
    end

    local func= select(n,...)
    if type(func)~="function" then
        error( "Last parameter not function: "..tostring(func) )
    end

    -- Check 'libs' already here, so the error goes in the right place
    -- (otherwise will be noticed only once the generator is called)
    --
    if libs then
        for s in string_gmatch(libs, "[%a*]+") do
            if not valid_libs[s] then
                error( "Bad library name: "..s )
            end
        end
    end
    
    local prio, cs, g_tbl

    for k,v in pairs(opt) do
            if k=="priority" then prio= v
        elseif k=="cancelstep" then cs= (v==true) and 100 or
                                        (v==false) and 0 or 
                                        type(v)=="number" and v or
                                        error( "Bad cancelstep: "..tostring(v), lev )
        elseif k=="globals" then g_tbl= v
        --..
        elseif k==1 then error( "unkeyed option: ".. tostring(v), lev )
        else error( "Bad option: ".. tostring(k), lev )
        end
    end

    -- Lane generator
    --
    return function(...)
              return lane_proxy( thread_new( func, libs, cs, prio, g_tbl,
                                             ... ) )     -- args
           end
end

lane_proxy= function( ud )
    local proxy= {
        _ud= ud,
        
        -- void= me:cancel()
        --
        cancel= function(me) thread_cancel(me._ud) end,
        
        -- [...] | [nil,err,stack_tbl]= me:join( [wait_secs=-1] )
        --
        join= function( me, wait ) 
                return thread_join( me._ud, wait )
            end,
        }
    assert( proxy._ud )
    setmetatable( proxy, lane_mt )

    return proxy
end


---=== Lindas ===---

-- We let the C code attach methods to userdata directly

-----
-- linda_ud= lanes.linda()
--
function linda()
    local proxy= _deep_userdata( linda_id )
    assert( (type(proxy) == "userdata") and getmetatable(proxy) )
    return proxy
end


---=== Timers ===---

--
-- On first 'require "lanes"', a timer lane is spawned that will maintain
-- timer tables and sleep in between the timer events. All interaction with
-- the timer lane happens via a 'timer_gateway' Linda, which is common to
-- all that 'require "lanes"'.
-- 
-- Linda protocol to timer lane:
--
--  TGW_KEY: linda_h, key, [wakeup_at_secs], [repeat_secs]
--
local TGW_KEY= "(timer control)"    -- the key does not matter, a 'weird' key may help debugging
local first_time_key= "first time"

local first_time= timer_gateway:get(first_time_key) == nil
timer_gateway:set(first_time_key,true)

--
-- Timer lane; initialize only on the first 'require "lanes"' instance (which naturally
-- has 'table' always declared)
--
if first_time then
    local table_remove= assert( table.remove )
    local table_insert= assert( table.insert )

    --
    -- { [deep_linda_lightuserdata]= { [deep_linda_lightuserdata]=linda_h, 
    --                                 [key]= { wakeup_secs [,period_secs] } [, ...] },
    -- }
    --
    -- Collection of all running timers, indexed with linda's & key.
    --
    -- Note that we need to use the deep lightuserdata identifiers, instead
    -- of 'linda_h' themselves as table indices. Otherwise, we'd get multiple
    -- entries for the same timer.
    --
    -- The 'hidden' reference to Linda proxy is used in 'check_timers()' but
    -- also important to keep the Linda alive, even if all outside world threw
    -- away pointers to it (which would ruin uniqueness of the deep pointer).
    -- Now we're safe.
    --
    local collection= {}

    --
    -- set_timer( linda_h, key [,wakeup_at_secs [,period_secs]] )
    --
    local function set_timer( linda, key, wakeup_at, period )

        assert( wakeup_at==nil or wakeup_at>0.0 )
        assert( period==nil or period>0.0 )

        local linda_deep= linda:deep()
        assert( linda_deep )

        -- Find or make a lookup for this timer
        --
        local t1= collection[linda_deep]
        if not t1 then
            t1= { [linda_deep]= linda }     -- proxy to use the Linda
            collection[linda_deep]= t1
        end
    
        if wakeup_at==nil then
            -- Clear the timer
            --
            t1[key]= nil

            -- Remove empty tables from collection; speeds timer checks and
            -- lets our 'safety reference' proxy be gc:ed as well.
            --
            local empty= true
            for k,_ in pairs(t1) do
                if k~= linda_deep then
                    empty= false; break
                end
            end
            if empty then
                collection[linda_deep]= nil
            end
            
            -- Note: any unread timer value is left at 'linda[key]' intensionally;
            --       clearing a timer just stops it.
        else
            -- New timer or changing the timings
            --
            local t2= t1[key]
            if not t2 then
                t2= {}; t1[key]= t2
            end
    
            t2[1]= wakeup_at
            t2[2]= period   -- can be 'nil'
        end
    end

    -----
    -- [next_wakeup_at]= check_timers()
    --
    -- Check timers, and wake up the ones expired (if any)
    --
    -- Returns the closest upcoming (remaining) wakeup time (or 'nil' if none).
    --
    local function check_timers()

        local now= now_secs()
        local next_wakeup

        for linda_deep,t1 in pairs(collection) do
            for key,t2 in pairs(t1) do
                --
                if key==linda_deep then
                    -- no 'continue' in Lua :/
                else
                    -- 't2': { wakeup_at_secs [,period_secs] }
                    --
                    local wakeup_at= t2[1]
                    local period= t2[2]     -- may be 'nil'
                    
                    if wakeup_at <= now then    
                        local linda= t1[linda_deep]
                        assert(linda)
    
                        linda:set( key, now )
        
                        -- 'pairs()' allows the values to be modified (and even
                        -- removed) as far as keys are not touched
                                                
                        if not period then
                            -- one-time timer; gone
                            --
                            t1[key]= nil
                            wakeup_at= nil   -- no 'continue' in Lua :/
                        else
                            -- repeating timer; find next wakeup (may jump multiple repeats)
                            --
                            repeat
                                wakeup_at= wakeup_at+period
                            until wakeup_at > now
    
                            t2[1]= wakeup_at
                        end
                    end
                    
                    if wakeup_at and ((not next_wakeup) or (wakeup_at < next_wakeup)) then
                        next_wakeup= wakeup_at
                    end 
                end
            end -- t2 loop
        end -- t1 loop
        
        return next_wakeup  -- may be 'nil'
    end    

    -----
    -- Snore loop (run as a lane on the background)
    --
    -- High priority, to get trustworthy timings.
    --
    -- We let the timer lane be a "free running" thread; no handle to it
    -- remains.
    --
    gen( "io", { priority=max_prio }, function()

        while true do
            local next_wakeup= check_timers()

            -- Sleep until next timer to wake up, or a set/clear command
            --
            local secs= next_wakeup and (next_wakeup - now_secs()) or nil
            local linda= timer_gateway:receive( secs, TGW_KEY )

            if linda then
                local key= timer_gateway:receive( 0.0, TGW_KEY )
                local wakeup_at= timer_gateway:receive( 0.0, TGW_KEY )
                local period= timer_gateway:receive( 0.0, TGW_KEY )
                assert( key and wakeup_at and period )

                set_timer( linda, key, wakeup_at, period>0 and period or nil )
            end
        end
    end )()
end

-----
-- = timer( linda_h, key_val, date_tbl|first_secs [,period_secs] )
--
function timer( linda, key, a, period )

    if a==0.0 then
        -- Caller expects to get current time stamp in Linda, on return
        -- (like the timer had expired instantly); it would be good to set this
        -- as late as possible (to give most current time) but also we want it
        -- to precede any possible timers that might start striking.
        --
        linda:set( key, now_secs() )

        if not period or period==0.0 then
            timer_gateway:send( TGW_KEY, linda, key, nil, nil )   -- clear the timer
            return  -- nothing more to do
        end
        a= period
    end

    local wakeup_at= type(a)=="table" and wakeup_conv(a)    -- given point of time
                                       or now_secs()+a
    -- queue to timer
    --
    timer_gateway:send( TGW_KEY, linda, key, wakeup_at, period )
end


---=== Lock & atomic generators ===---

-- These functions are just surface sugar, but make solutions easier to read.
-- Not many applications should even need explicit locks or atomic counters.

--
-- lock_f= lanes.genlock( linda_h, key [,N_uint=1] )
--
-- = lock_f( +M )   -- acquire M
--      ...locked...
-- = lock_f( -M )   -- release M
--
-- Returns an access function that allows 'N' simultaneous entries between
-- acquire (+M) and release (-M). For binary locks, use M==1.
--
function genlock( linda, key, N )
    linda:limit(key,N)
    linda:set(key,nil)  -- clears existing data

    --
    -- [true [, ...]= trues(uint)
    --
    local function trues(n)
        if n>0 then return true,trues(n-1) end
    end

    return
    function(M)
        if M>0 then
            -- 'nil' timeout allows 'key' to be numeric
            linda:send( nil, key, trues(M) )    -- suspends until been able to push them
        else
            for i=1,-M do
                linda:receive( key )
            end
        end
    end
end


--
-- atomic_f= lanes.genatomic( linda_h, key [,initial_num=0.0] )
--
-- int= atomic_f( [diff_num=1.0] )
--
-- Returns an access function that allows atomic increment/decrement of the
-- number in 'key'.
--
function genatomic( linda, key, initial_val )
    linda:limit(key,2)          -- value [,true]
    linda:set(key,initial_val or 0.0)   -- clears existing data (also queue)

    return
    function(diff)
        -- 'nil' allows 'key' to be numeric
        linda:send( nil, key, true )    -- suspends until our 'true' is in
        local val= linda:get(key) + (diff or 1.0)
        linda:set( key, val )   -- releases the lock, by emptying queue
        return val
    end
end


--the end
