/* 
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

*/

static char * keynames[] = {
	"KEY_INT",
	"KEY_STRING",
	"KEY_VERSION", // Best practice to have a version number as the last persistent storage
	"KEY_COUNT" // Must have this as last key name.  Replace the first 2 above with your key names.
};

static int version = 1;
static bool dbg = false; // False only reports real errors, true logs all actions.

typedef struct {
	int type;
	int intValue;
	int newIntValue;
	char stringValue[80];
	char newStringValue[80];
} perm_cfg;

static perm_cfg perm[KEY_COUNT];

enum {READ, WRITE, VERIFY};

int perm_int(int mode, int key, int value) {
	switch (mode) {
		case READ:
			perm[key].intValue = persist_read_int(key);
			perm[key].newIntValue = perm[key].intValue;
 			if(dbg)APP_LOG(APP_LOG_LEVEL_DEBUG, "Read Int key %s is: %d", keynames[key], perm[key].intValue);
			
			break;
		case WRITE:
			perm[key].newIntValue = value;
			int status = persist_write_int(key, perm[key].newIntValue);
			perm[key].intValue = persist_read_int(key);
			if (perm[key].intValue  != perm[key].newIntValue) {
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "Persist int failed with status %d. Key %s was %d, should be %d", status, keynames[key], perm[key].intValue, perm[key].newIntValue);
			} else {
			 	if(dbg)APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote int Key %s is %d", keynames[key], perm[key].intValue);
			}
			break;
		case VERIFY:
			perm[key].intValue = persist_read_int(key);
			if (perm[key].intValue  != perm[key].newIntValue) {
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "Verify int failed. Key %s was %d, should be %d", keynames[key], perm[key].intValue, perm[key].newIntValue);
				persist_write_int(key, perm[key].newIntValue);
			}
			break;
	}
	return perm[key].intValue;
}

int perm_string(int mode, int key, char *string, int len) {
	int numBytes = E_DOES_NOT_EXIST;
	switch (mode) {
		case READ:
			numBytes = persist_read_string(key, perm[key].stringValue, len);
			if (numBytes == E_DOES_NOT_EXIST) {
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "String key %s doesn't exist", keynames[key]);
			} else {
				strcpy(string, perm[key].stringValue);
				strcpy(perm[key].newStringValue, perm[key].stringValue);
 				if(dbg)APP_LOG(APP_LOG_LEVEL_DEBUG, "Read String key %s is: %s", keynames[key], perm[key].stringValue);
			}
			break;
		case WRITE:
			strcpy(perm[key].newStringValue, string);
			persist_write_string(key, perm[key].newStringValue);
			numBytes = persist_read_string(key, perm[key].stringValue, sizeof(perm[key].stringValue));
			if (numBytes == E_DOES_NOT_EXIST) {
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "String persist failed. Key %s doesn't exist", keynames[key]);
			} else {
				if (strcmp(perm[key].stringValue, perm[key].newStringValue) != 0) {
					APP_LOG(APP_LOG_LEVEL_DEBUG, "String persist failed. Key %s was %s, should be %s", keynames[key], perm[key].stringValue, perm[key].newStringValue);
				} else {
 					if(dbg)APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote String key %s is: %s", keynames[key], string);
 				}
			}
			break;
		case VERIFY:
			numBytes = persist_read_string(key, perm[key].stringValue, sizeof(perm[key].stringValue));
			if (numBytes == E_DOES_NOT_EXIST) {
 				APP_LOG(APP_LOG_LEVEL_DEBUG, "String verify failed. Key %d doesn't exist", key);
				persist_write_string(key, perm[key].newStringValue);
			} else {
				if (strcmp(perm[key].stringValue, perm[key].newStringValue) != 0) {
					APP_LOG(APP_LOG_LEVEL_DEBUG, "String verify failed. Key %s was %s, should be %s", keynames[key], perm[key].stringValue, perm[key].newStringValue);
					persist_write_string(key, perm[key].newStringValue);
				}
			}
			break;
	}
	return numBytes;
}
