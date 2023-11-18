/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "ios.h"

#ifdef LOVE_IOS

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import <AudioToolbox/AudioServices.h>
#import <AVFoundation/AVFoundation.h>

#include "modules/audio/Audio.h"

#include <vector>

#include <SDL_events.h>
#include <SDL_video.h>
#include <SDL_syswm.h>

static NSArray *getLovesInDocuments();
static bool deleteFileInDocuments(NSString *filename);

@interface LOVETableViewController : UITableViewController

- (instancetype)initWithGameList:(NSArray *)list;

@property (nonatomic) NSMutableArray *gameList;
@property (nonatomic, readonly, copy) NSString *selectedGame;

@end

@implementation LOVETableViewController

- (instancetype)initWithGameList:(NSArray *)list
{
	if ((self = [super init]))
	{
		_gameList = [[NSMutableArray alloc] initWithArray:list copyItems:YES];

		self.title = @"LÖVE Games";
		self.navigationItem.rightBarButtonItem = self.editButtonItem;
	}

	return self;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	#pragma unused(tableView)
	#pragma unused(section)
	// We want to list all games plus the no-game screen.
	return self.gameList.count + 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *cellIdentifier = @"LOVETableCell";

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
	if (cell == nil)
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];

	if (indexPath.row < (NSInteger) self.gameList.count)
		cell.textLabel.text = self.gameList[indexPath.row];
	else
		cell.textLabel.text = @"No-game screen";

	return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	#pragma unused(tableView)
	if (indexPath.row < (NSInteger) self.gameList.count)
		_selectedGame = [(NSString *)(self.gameList[indexPath.row]) copy];
	else
	{
		// We test against nil to check if a game has been selected, so we'll
		// just use an empty string instead to represent the no-game screen.
		_selectedGame = @"";
	}
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
	if (editingStyle != UITableViewCellEditingStyleDelete)
		return;

	if (indexPath.row >= (NSInteger) self.gameList.count)
		return;

	NSString *filename = self.gameList[indexPath.row];

	// Delete the file.
	if (deleteFileInDocuments(filename))
	{
		[self.gameList removeObjectAtIndex:indexPath.row];
		[tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
	}
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath;
{
	#pragma unused(tableView)
	// The no-game screen isn't removable.
	return indexPath.row < (NSInteger) self.gameList.count;
}

@end

static NSString *getDocumentsDirectory()
{
	NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	return docdirs[0];
}

static NSArray *getLovesInDocuments()
{
	NSMutableArray *paths = [NSMutableArray new];

	NSFileManager *manager = [NSFileManager defaultManager];
	NSDirectoryEnumerator *enumerator = [manager enumeratorAtPath:getDocumentsDirectory()];

	NSString *path = nil;
	while ((path = [enumerator nextObject]))
	{
		//  Add .love files plus folders that contain main.lua to our list.
		if ([path.pathExtension isEqualToString:@"love"])
			[paths addObject:path];
		else if ([path.lastPathComponent isEqualToString:@"main.lua"])
			[paths addObject:path.stringByDeletingLastPathComponent];
	}

	[paths sortUsingSelector:@selector(localizedCaseInsensitiveCompare:)];

	return paths;
}

static bool deleteFileInDocuments(NSString *filename)
{
	NSString *documents = getDocumentsDirectory();

	NSString *file = [documents stringByAppendingPathComponent:filename];
	bool success = [[NSFileManager defaultManager] removeItemAtPath:file error:nil];

	if (success)
		NSLog(@"Deleted file %@ in Documents folder.", filename);

	return success;
}

static int dropFileEventFilter(void *userdata, SDL_Event *event)
{
	@autoreleasepool
	{
		if (event->type != SDL_DROPFILE)
			return 1;

		NSString *fname = @(event->drop.file);
		NSFileManager *fmanager = [NSFileManager defaultManager];

		if ([fmanager fileExistsAtPath:fname] && [fname.pathExtension isEqual:@"love"])
		{
			NSString *documents = getDocumentsDirectory();

			documents = documents.stringByStandardizingPath.stringByResolvingSymlinksInPath;
			fname = fname.stringByStandardizingPath.stringByResolvingSymlinksInPath;

			// Is the file inside the Documents directory?
			if ([fname hasPrefix:documents])
			{
				LOVETableViewController *vc = (__bridge LOVETableViewController *) userdata;

				// Update the game list.
				NSArray *games = getLovesInDocuments();
				vc.gameList = [[NSMutableArray alloc] initWithArray:games copyItems:YES];
				[vc.tableView reloadData];

				SDL_free(event->drop.file);
				return 0;
			}
		}

		return 1;
	}
}

@interface LoveAudioInterruptionListener : NSObject
@end

@implementation LoveAudioInterruptionListener

+ (id) shared
{
	// thread-safe singleton
	static dispatch_once_t pred = 0;
	__strong static id _shared = nil;
	dispatch_once(&pred, ^{
			_shared = [[self alloc] init];
		});
	return _shared;
}

- (void)audioSessionInterruption:(NSNotification *)note
{
	@synchronized (self)
	{
		auto audio = love::Module::getInstance<love::audio::Audio>(love::Module::M_AUDIO);
		if (!audio)
		{
			NSLog(@"LoveAudioInterruptionListener could not get love audio module");
			return;
		}
		NSNumber *type = note.userInfo[AVAudioSessionInterruptionTypeKey];
		if (type.unsignedIntegerValue == AVAudioSessionInterruptionTypeBegan)
			audio->pauseContext();
		else
			audio->resumeContext();
	}
}

- (void)applicationBecameActive:(NSNotification *)note
{
	@synchronized (self)
	{
		auto audio = love::Module::getInstance<love::audio::Audio>(love::Module::M_AUDIO);
		if (!audio)
		{
			NSLog(@"ERROR:could not get love audio module");
			return;
		}
		audio->resumeContext();
	}
}

@end // LoveAudioInterruptionListener

namespace love
{
namespace ios
{

/**
 * Displays a full-screen list of available LOVE games for the user to choose.
 * Returns the index of the selected game from the list. The list of games
 * includes the no-game screen, and the function will return an index outside
 * of the array's range if that is selected.
 **/
static NSString *showGameList(NSArray *filenames)
{
	// Game list view controller.
	LOVETableViewController *tablecontroller = [[LOVETableViewController alloc] initWithGameList:filenames];

	// Navigation view controller (only used for the header bar right now.)
	// Contains the game list view/controller.
	UINavigationController *navcontroller = [[UINavigationController alloc] initWithRootViewController:tablecontroller];

	UIWindow *window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	window.rootViewController = navcontroller;

	SDL_EventFilter oldfilter = nullptr;
	void *oldudata = nullptr;
	SDL_GetEventFilter(&oldfilter, &oldudata);

	// Manually retain the table VC and use it for the event filter userdata.
	// We need to set a custom event filter to update the table when .love files
	// are opened by the user.
	void *tableudata = (void *) CFBridgingRetain(tablecontroller);
	SDL_SetEventFilter(dropFileEventFilter, tableudata);

	[window makeKeyAndVisible];

	// Process events until a game in the list is selected.
	NSRunLoop *runloop = [NSRunLoop currentRunLoop];
	while (tablecontroller.selectedGame == nil)
	{
		[runloop runMode:NSDefaultRunLoopMode  beforeDate:[NSDate distantPast]];
		[runloop runMode:UITrackingRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:1.0/60.0]];
	}

	// The window will get released and cleaned up once we go out of scope.
	window.hidden = YES;

	SDL_SetEventFilter(oldfilter, oldudata);
	CFBridgingRelease(tableudata);

	return tablecontroller.selectedGame;
}

std::string getLoveInResources(bool &fused)
{
	fused = false;
	std::string path;

	@autoreleasepool
	{
		// Start by looking in the main bundle (.app) folder for .love files.
		NSArray *bundlepaths = [[NSBundle mainBundle] pathsForResourcesOfType:@"love" inDirectory:nil];

		if (bundlepaths.count > 0)
		{
			// The game should be fused if we have something here.
			fused = true;
			return [bundlepaths[0] UTF8String];
		}

		// Otherwise look in the app's Documents directory. The game won't be
		// fused.
		NSArray *filepaths = getLovesInDocuments();

		// Let the user select a game from the un-fused list.
		NSString *selectedfile = showGameList(filepaths);

		// The string length might be 0 if the no-game screen was selected.
		if (selectedfile != nil && selectedfile.length > 0)
		{
			NSString *documents = getDocumentsDirectory();
			path = [documents stringByAppendingPathComponent:selectedfile].UTF8String;
		}
	}

	return path;
}

std::string getAppdataDirectory()
{
	NSSearchPathDirectory searchdir = NSApplicationSupportDirectory;
	std::string path;

	@autoreleasepool
	{
		NSArray *dirs = NSSearchPathForDirectoriesInDomains(searchdir, NSUserDomainMask, YES);

		if (dirs.count > 0)
			path = [dirs[0] UTF8String];
	}

	return path;
}

std::string getHomeDirectory()
{
	std::string path;

	@autoreleasepool
	{
		path = [NSHomeDirectory() UTF8String];
	}

	return path;
}

bool openURL(const std::string &url)
{
	bool success = false;

	@autoreleasepool
	{
		UIApplication *app = [UIApplication sharedApplication];
		NSURL *nsurl = [NSURL URLWithString:@(url.c_str())];

		if ([app canOpenURL:nsurl])
			success = [app openURL:nsurl];
	}

	return success;
}

std::string getExecutablePath()
{
	@autoreleasepool
	{
		return std::string([NSBundle mainBundle].executablePath.UTF8String);
	}
}

void vibrate()
{
	@autoreleasepool
	{
		AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
	}
}

bool setAudioMixWithOthers(bool mixEnabled)
{
	@autoreleasepool
	{
		NSString *category = AVAudioSessionCategorySoloAmbient;
		NSError *err;

		if (mixEnabled)
			category = AVAudioSessionCategoryAmbient;

		bool success = [[AVAudioSession sharedInstance] setCategory:category error:&err];
		if (!success)
			NSLog(@"Error in AVAudioSession setCategory: %@", [err localizedDescription]);

		return success;
	}
}

bool hasBackgroundMusic()
{
	@autoreleasepool
	{
		AVAudioSession *session = [AVAudioSession sharedInstance];
		if ([session respondsToSelector:@selector(secondaryAudioShouldBeSilencedHint)])
			return session.secondaryAudioShouldBeSilencedHint;
		return false;
	}
}

void initAudioSessionInterruptionHandler()
{
	@autoreleasepool
	{
		AVAudioSession *session = [AVAudioSession sharedInstance];
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

		[center addObserver:[LoveAudioInterruptionListener shared]
			   selector:@selector(audioSessionInterruption:)
			       name:AVAudioSessionInterruptionNotification
			     object:session];

		// An interruption end notification is not guaranteed to be sent if
		// we were previously interrupted... resuming if needed when the app
		// becomes active seems to be the way to go.
		[center addObserver:[LoveAudioInterruptionListener shared]
			   selector:@selector(applicationBecameActive:)
			       name:UIApplicationDidBecomeActiveNotification
			     object:nil];

		[center addObserver:[LoveAudioInterruptionListener shared]
			   selector:@selector(applicationBecameActive:)
			       name:UIApplicationWillEnterForegroundNotification
			     object:nil];
	}
}

void destroyAudioSessionInterruptionHandler()
{
	[[NSNotificationCenter defaultCenter] removeObserver:[LoveAudioInterruptionListener shared]];
}

Rect getSafeArea(SDL_Window *window)
{
	@autoreleasepool
	{
		Rect rect = {};
		SDL_GetWindowSize(window, &rect.w, &rect.h);

		SDL_SysWMinfo info = {};
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(window, &info))
		{
			UIView *view = info.info.uikit.window.rootViewController.view;
			if (@available(iOS 11.0, tvOS 11.0, *))
			{
				UIEdgeInsets insets = view.safeAreaInsets;

				rect.x += insets.left;
				rect.w -= insets.left;

				rect.w -= insets.right;

				rect.y += insets.top;
				rect.h -= insets.top;

				rect.h -= insets.bottom;
			}
		}

		return rect;
	}
}

} // ios
} // love

#endif // LOVE_IOS
