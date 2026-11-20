/*
 * eclair - Android backends: AccessibilityManager (screen reader), TextToSpeech (synthesizer)
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 */

package org.eclair;

import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.Context;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;

import java.nio.charset.StandardCharsets;
import java.util.List;

public final class Eclair {

	static {
		try {
			System.loadLibrary("eclair");
		} catch (UnsatisfiedLinkError e) {
			// do nothing
		}
	}

	// Written by EclairInitProvider on the main thread
	private static volatile Context context;

	private static volatile TextToSpeech tts;
	private static volatile boolean ttsReady;

	private static volatile float volume = 1.0f;

	private Eclair() {}

	public static void setContext(Context ctx) {
		context = ctx;
	}

	/* ---------------------------------------------------------------
	 * lifecycle
	 * --------------------------------------------------------------- */

	static boolean init() {
		Context ctx = context;

		// if we have no context, the backends should just report unavailable
		if (ctx == null) {
			return true;
		}

		tts = new TextToSpeech(ctx, new TextToSpeech.OnInitListener() {
			@Override
			public void onInit(int status) {
				TextToSpeech t = tts;
				if (status != TextToSpeech.SUCCESS || t == null) {
					return;
				}

				ttsReady = true;
			}
		});

		return true;
	}

	static void shutdown() {
		TextToSpeech t = tts;

		tts = null;
		ttsReady = false;

		if (t != null) {
			t.stop();
			t.shutdown();
		}
	}

	/* ---------------------------------------------------------------
	 * screen reader route - AccessibilityManager
	 * --------------------------------------------------------------- */

	static boolean srAvailable() {
		AccessibilityManager am = manager();
		if (am == null | !am.isEnabled()) {
			return false;
		}

		// accessibility manager can be enabled without a speaking / brailled service,
		// so we check here if we have one enabled.
		List<AccessibilityServiceInfo> srServices = am.getEnabledAccessibilityServiceList(
			AccessibilityServiceInfo.FEEDBACK_SPOKEN | AccessibilityServiceInfo.FEEDBACK_BRAILLE
		);

		boolean hasSrServicesEnabled = !srServices.isEmpty();
		return hasSrServicesEnabled;
	}

	static boolean srSpeak(byte[] utf8, boolean interrupt) {
		AccessibilityManager am = manager();
		Context ctx = context;

		if (am == null || ctx == null || !am.isEnabled()) {
			return false;
		}

		String text = new String(utf8, StandardCharsets.UTF_8);

		if (interrupt) {
			am.interrupt();
		}

		// TYPE_ANNOUNCEMENT is deprecated - there doesn't appear to be any good alternatives
		// for plain text announcements that don't reference a specific node
		AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_ANNOUNCEMENT);
		event.setPackageName(ctx.getPackageName());
		event.setClassName(Eclair.class.getName());
		event.getText().add(text);

		am.sendAccessibilityEvent(event);
		return true;
	}

	static boolean srStop() {
		AccessibilityManager am = manager();
		if (am == null) {
			return false;
		}

		am.interrupt();
		return true;
	}

	/* ---------------------------------------------------------------
	 * synthesizer route - TextToSpeech
	 * --------------------------------------------------------------- */

	static boolean synthAvailable() {
		return ttsReady;
	}

	static boolean synthSpeak(byte[] utf8, boolean interrupt) {
		TextToSpeech t = tts;
		if (t == null || !ttsReady) {
			return false;
		}

		String text = new String(utf8, StandardCharsets.UTF_8);

		Bundle params = new Bundle();
		params.putFloat(TextToSpeech.Engine.KEY_PARAM_VOLUME, volume);

		int queue = interrupt ? TextToSpeech.QUEUE_FLUSH : TextToSpeech.QUEUE_ADD;

		return t.speak(text, queue, params, null) == TextToSpeech.SUCCESS;
	}

	static boolean synthStop() {
		TextToSpeech t = tts;
		if (t == null) {
			return false;
		}

		return t.stop() == TextToSpeech.SUCCESS;
	}

	static void synthSetRate(float nativeRate) {
		TextToSpeech t = tts;
		if (t != null) {
			t.setSpeechRate(nativeRate);
		}
	}

	static void synthSetVolume(float nativeVolume) {
		volume = nativeVolume;
	}

	/* ---------------------------------------------------------------
	 * helpers
	 * --------------------------------------------------------------- */

	private static AccessibilityManager manager() {
		Context ctx = context;
		if (ctx == null) {
			return null;
		}

		return (AccessibilityManager) ctx.getSystemService(Context.ACCESSIBILITY_SERVICE);
	}
}
