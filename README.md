	perm_int/perm_string wrappers for Pebble's persist_* api.
	usage examples:

	currentInt = perm_int(READ, KEY_INT, currentInt);
	currentInt = perm_int(WRITE, KEY_INT, newInt);
	perm_int(WRITE, KEY_INT, newInt);
	
	if (persist_exists(KEY_STRING))
		perm_string(READ, KEY_STRING, myString, sizeof(myString));
	else {
		strcpy(myString, "default value");
		perm_string(WRITE, KEY_STRING, myString, sizeof(myString));
	}
	
	if (perm_int(READ, KEY_VERSION, version) != version) {
 		APP_LOG(APP_LOG_LEVEL_DEBUG, "Version reset (was v %d, now v %d)", perm_int(READ, AKEY_VERSION, version), version);		
		for (int key=0; key <= KEY_VERSION; key++) {
			int status = persist_exists(key) ? persist_delete(key) : E_DOES_NOT_EXIST;
 			if(status != E_DOES_NOT_EXIST && status != S_SUCCESS)
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "Deleted key %s, status=%d", keynames[key], status);		
		}
		perm_int(WRITE, AKEY_VERSION, version);
	}

