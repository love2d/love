package org.love2d.android;

import java.util.List;

import android.app.DownloadManager;
import android.app.IntentService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

public class DownloadService extends IntentService {
    public DownloadService() {
        super("DownloadService");
    }

    @Override
    public void onDestroy() {
        Log.d("DownloadService", "destroying");
        unregisterReceiver(downloadReceiver);
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Log.d("DownloadService", "service started");

        String url = intent.getStringExtra("url");
        Uri uri = Uri.parse(url);

        Log.d("DownloadService", "Downloading from url: " + url + "file = " + uri.getLastPathSegment());

        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url));
        request.setDescription("LÖVE Game Download");
        request.setTitle(uri.getLastPathSegment());
        request.setMimeType("application/x-love-game");

        // in order for this if to run, you must use the android 3.2 to compile your app
        if (Build.VERSION.SDK_INT >= 11) {
            DownloadRequestSettings_API11 settings = new DownloadRequestSettings_API11();
            settings.setup(request);
        }
        request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, uri.getLastPathSegment());
        // get download service and enqueue file

        Log.d("DownloadActivity", "creating manager");
        DownloadManager manager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);
        Log.d("DownloadActivity", "enqueuing download");
        manager.enqueue(request);

        Log.d("DownloadActivity", "download receiver = " + downloadReceiver);
        IntentFilter intentFilter = new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
        registerReceiver(downloadReceiver, intentFilter);
    }

    /**
     * @param context used to check the device version and DownloadManager information
     * @return true if the download manager is available
     */
    public static boolean isDownloadManagerAvailable(Context context) {
        try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.GINGERBREAD) {
                return false;
            }
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setClassName("com.android.providers.downloads.ui", "com.android.providers.downloads.ui.DownloadList");
            List<ResolveInfo> list = context.getPackageManager().queryIntentActivities(intent,
                    PackageManager.MATCH_DEFAULT_ONLY);
            return list.size() > 0;
        } catch (Exception e) {
            return false;
        }
    }

    private BroadcastReceiver downloadReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d("DownloadActivity", "downloadReceiver intent called");

        }
    };
}

class DownloadRequestSettings_API11 {
    public static void setup(DownloadManager.Request request) {
        request.allowScanningByMediaScanner();
        request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
    }
}
