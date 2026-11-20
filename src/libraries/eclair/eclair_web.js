/*
 * eclair - Web backend implementation
 * Copyright (c) 2026 Jesse Jurman. zlib license - see LICENSE.md
 *
 * Every function returns 1 or 0 rather than true or false. A C bool arrives as
 * an i32, and returning a JS boolean leaves the coercion to chance.
 */

mergeInto(LibraryManager.library, {
	$eclairWebState: {
		rate: 1.0,
		volume: 1.0
	},

	/* ------------------------------------------------- *
	 * screen reader route - ariaNotify
	 * ------------------------------------------------- */

	eclair_js_sr_speak__deps: ['$UTF8ToString'],
	eclair_js_sr_speak: function (utf8, interrupt) {
		try {
			document.ariaNotify(UTF8ToString(utf8), {
				priority: interrupt ? "high" : "normal"
			});
			return 1;
		} catch (e) {
			// note - we can end up here when we have no document (e.g. web-worker)
			return 0;
		}
	},

	/* ------------------------------------------------- *
	 * synthesizer route - Web Speech
	 * ------------------------------------------------- */

	eclair_js_synth_available: function () {
		return (typeof speechSynthesis !== "undefined") ? 1 : 0;
	},

	eclair_js_synth_speak__deps: ['$UTF8ToString', '$eclairWebState'],
	eclair_js_synth_speak: function (utf8, interrupt) {
		if (typeof speechSynthesis === "undefined") {
			return 0;
		}

		try {
			if (interrupt) {
				speechSynthesis.cancel();
			}

			var utterance = new SpeechSynthesisUtterance(UTF8ToString(utf8));
			utterance.rate = eclairWebState.rate;
			utterance.volume = eclairWebState.volume;

			speechSynthesis.speak(utterance);
			return 1;
		} catch (e) {
			return 0;
		}
	},

	eclair_js_synth_stop: function () {
		if (typeof speechSynthesis === "undefined") {
			return 0;
		}

		speechSynthesis.cancel();
		return 1;
	},

	eclair_js_synth_set_rate__deps: ['$eclairWebState'],
	eclair_js_synth_set_rate: function (rate) {
		eclairWebState.rate = rate;
	},

	eclair_js_synth_set_volume__deps: ['$eclairWebState'],
	eclair_js_synth_set_volume: function (volume) {
		eclairWebState.volume = volume;
	}

})
