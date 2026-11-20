/*
 * eclair - Android context bootstrap
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

package org.eclair;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

/**
 * EclairInitProvider - Context Provider that bootstraps the library
 * ahead of the application code.
 */
public final class EclairInitProvider extends ContentProvider {

	@Override
	public boolean onCreate() {
		// first reference to class, so forces the JVM to run during JVM initialization
		Context context = getContext();

		// application context rather than the provider
		if (context != null) {
			Eclair.setContext(context.getApplicationContext());
		}

		return true;
	}

	// None of these are called, implemented as no-ops

	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
											String[] selectionArgs, String sortOrder) {
		return null;
	}

	@Override
	public String getType(Uri uri) {
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		return null;
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		return 0;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
										String[] selectionArgs) {
		return 0;
	}
}
