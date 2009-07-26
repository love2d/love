--
-- KEEPER.LUA
--
-- Keeper state logic
--
-- This code is read in for each "keeper state", which are the hidden, inter-
-- mediate data stores used by Lanes inter-state communication objects.
--
-- Author: Asko Kauppi <akauppi@gmail.com>
--
--[[
===============================================================================

Copyright (C) 2008 Asko Kauppi <akauppi@gmail.com>

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

-- unique key instead of 'nil' in queues
--
assert( nil_sentinel )

-- We only need to have base and table libraries (and io for debugging)
--
local table_remove= assert( table.remove )
local table_concat= assert( table.concat )

local function WR(...)
    if io then 
        io.stderr:write( table_concat({...},'\t').."\n" ) 
    end
end

-----
-- Actual data store
--
-- { [linda_deep_ud]= { key= val [, ...] }
--      ...
-- }
--
local _data= {}

-----
-- Entries queued for use when the existing 'data[ud][key]' entry is consumed.
--
-- { [linda_deep_ud]= { key= { val [, ... } [, ...] }
--      ...
-- }
--
local _incoming= {}

-----
-- Length limits (if any) for queues
--
-- 0:   don't queue values at all; ':send()' waits if the slot is not vacant
-- N:   allow N values to be queued (slot itself + N-1); wait if full
-- nil: no limits, '_incoming' may grow endlessly
--
local _limits= {}

-----
-- data_tbl, incoming_tbl, limits_tbl = tables( linda_deep_ud )
--
-- Gives appropriate tables for a certain Linda (creates them if needed)
--
local function tables( ud )
    -- tables are created either all or nothing
    --
    if not _data[ud] then
        _data[ud]= {}
        _incoming[ud]= {}
        _limits[ud]= {}
    end
    return _data[ud], _incoming[ud], _limits[ud]
end


local function DEBUG(title,ud,key)
    assert( title and ud and key )

    local data,incoming,_= tables(ud)

    local s= tostring(data[key])
    for _,v in ipairs( incoming[key] or {} ) do
        s= s..", "..tostring(v)
    end
    WR( "*** "..title.." ("..tostring(key).."): ", s )
end


-----
-- bool= send( linda_deep_ud, key, ... )
--
-- Send new data (1..N) to 'key' slot. This send is atomic; all the values
-- end up one after each other (this is why having possibility for sending
-- multiple values in one call is deemed important).
--
-- If the queue has a limit, values are sent only if all of them fit in.
--
-- Returns: 'true' if all the values were placed
--          'false' if sending would exceed the queue limit (wait & retry)
--
function send( ud, key, ... )

    local data,incoming,limits= tables(ud)

    local n= select('#',...)
    if n==0 then return true end    -- nothing to send

    -- Initialize queue for all keys that have been used with ':send()'
    --
    if incoming[key]==nil then
        incoming[key]= {}
    end

    local len= data[key] and 1+#incoming[key] or 0
    local m= limits[key]

    if m and len+n > m then
        return false    -- would exceed the limit; try again later
    end

    for i=1,n do
        local val= select(i,...)

        -- 'nil' in the data replaced by sentinel
        if val==nil then
            val= nil_sentinel
        end

        if len==0 then
            data[key]= val
            len= 1
        else
            incoming[key][len]= val
            len= len+1
        end
    end
    return true
end


-----
-- [val, key]= receive( linda_deep_ud, key [, ...] )
--
-- Read any of the given keys, consuming the data found. Keys are read in
-- order.
--
function receive( ud, ... )

    local data,incoming,_= tables(ud)

    for i=1,select('#',...) do
        local key= select(i,...)
        local val= data[key]

        if val~=nil then
            if incoming[key] and incoming[key][1]~=nil then
                -- pop [1] from 'incoming[key]' into the actual slot
                data[key]= table_remove( incoming[key], 1 )
            else
                data[key]= nil  -- empty the slot
            end
            if val==nil_sentinel then
                val= nil
            end
            return val, key
        end
    end
    --return nil
end


-----
-- = limit( linda_deep_ud, key, uint )
--
function limit( ud, key, n )

    local _,_,limits= tables(ud)

    limits[key]= n
end


-----
-- void= set( linda_deep_ud, key, [val] )
--
function set( ud, key, val )

    local data,incoming,_= tables(ud)

    -- Setting a key to 'nil' really clears it; only queing uses sentinels.
    --
    data[key]= val
    incoming[key]= nil
end


-----
-- [val]= get( linda_deep_ud, key )
--
function get( ud, key )

    local data,_,_= tables(ud)

    local val= data[key]
    if val==nil_sentinel then
        val= nil
    end
    return val
end


-----
-- void= clear( linda_deep_ud )
--
-- Clear the data structures used for a Linda (at its destructor)
--
function clear( ud )

    _data[ud]= nil
    _incoming[ud]= nil
    _limits[ud]= nil
end


