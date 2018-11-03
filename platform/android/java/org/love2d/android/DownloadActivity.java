package org.love2d.android;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

public class DownloadActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Uri uri = this.getIntent().getData();

        if (uri.getScheme().equals("http")) {
            String url = uri.toString();
            Intent intent = new Intent(this, DownloadService.class);
            intent.putExtra("url", url);
            startService(intent);
        }

        finish();
    }
}
