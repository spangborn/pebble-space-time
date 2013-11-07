Pebble.addEventListener("ready", function(e) {
    Pebble.showSimpleNotificationOnPebble("TEST", "YOU SUCK BALLS.");
});
Pebble.addEventListener("showConfiguration", function (e) {
	Pebble.openURL("http://anesti.org/test.html");
});
