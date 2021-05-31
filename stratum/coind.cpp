
#include "stratum.h"

void coind_error(YAAMP_COIND *coind, const char *s)
{
	coind->auto_ready = false;

	object_delete(coind);
	debuglog("%s error %s\n", coind->name, s);
}

double coind_profitability(YAAMP_COIND *coind)
{
	if(!coind->difficulty) return 0;
	if(coind->pool_ttf > g_stratum_max_ttf) return 0;

//	double prof = 24*60*60*1000 / (coind->difficulty / 1000000 * 0x100000000) * reward * coind->price;
//	double prof = 24*60*60*1000 / coind->difficulty / 4294.967296 * reward * coind->price;

	double prof = 20116.56761169 / coind->difficulty * coind->reward * coind->price;
	if(!strcmp(g_current_algo->name, "sha256")) prof *= 1000;

	if(!coind->isaux && !coind->pos)
	{
		for(CLI li = g_list_coind.first; li; li = li->next)
		{
			YAAMP_COIND *aux = (YAAMP_COIND *)li->data;
			if(!coind_can_mine(aux, true)) continue;

			prof += coind_profitability(aux);
		}
	}

	return prof;
}

double coind_nethash(YAAMP_COIND *coind)
{
	double speed = coind->difficulty * 0x100000000 / 1000000 / max(min(coind->actual_ttf, 60), 30);
//	if(!strcmp(g_current_algo->name, "sha256")) speed *= 1000;

	return speed;
}

void coind_sort()
{
	for(CLI li = g_list_coind.first; li && li->next; li = li->next)
	{
		YAAMP_COIND *coind1 = (YAAMP_COIND *)li->data;
		if(coind1->deleted) continue;

		YAAMP_COIND *coind2 = (YAAMP_COIND *)li->next->data;
		if(coind2->deleted) continue;

		double p1 = coind_profitability(coind1);
		double p2 = coind_profitability(coind2);

		if(p2 > p1)
		{
			g_list_coind.Swap(li, li->next);
			coind_sort();

			return;
		}
	}
}

bool coind_can_mine(YAAMP_COIND *coind, bool isaux)
{
	if(coind->deleted) return false;
	if(!coind->enable) return false;
	if(!coind->auto_ready) return false;
	if(!rpc_connected(&coind->rpc)) return false;
	if(!coind->height) return false;
	if(!coind->difficulty) return false;
	if(coind->isaux != isaux) return false;
//	if(isaux && !coind->aux.chainid) return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool coind_validate_user_address(YAAMP_COIND *coind, char* const address)
{
	if(!address[0]) return false;

	char params[YAAMP_SMALLBUFSIZE];
	sprintf(params, "[\"%s\"]", address);

	json_value *json = rpc_call(&coind->rpc, "validateaddress", params);
	if(!json) return false;

	json_value *json_result = json_get_object(json, "result");
	if(!json_result) {
		json_value_free(json);
		return false;
	}

	bool isvalid = json_get_bool(json_result, "isvalid");
	// if(!isvalid) stratumlog("%s: %s user address %s is not valid.\n", g_stratum_algo, coind->symbol, address);

	json_value_free(json);

	return isvalid;
}

///////////////////////////////////////////////////////////////////////////////

bool coind_validate_address(YAAMP_COIND *coind)
{
	if(!coind->wallet[0]) return false;

	char params[YAAMP_SMALLBUFSIZE];
	sprintf(params, "[\"%s\"]", coind->wallet);

	json_value *json;
    bool getaddressinfo = ((strcmp(coind->symbol,"DGB") == 0) || (strcmp(coind->symbol2, "DGB") == 0) || (strcmp(coind->symbol,"PICOIN") == 0) || (strcmp(coind->symbol2, "PICOIN") == 0) || (strcmp(coind->symbol,"VVC") == 0) || (strcmp(coind->symbol2, "VVC") == 0) || (strcmp(coind->symbol,"PIC") == 0) || (strcmp(coind->symbol2, "PIC") == 0) || (strcmp(coind->symbol,"BNS") == 0) || (strcmp(coind->symbol2, "BNS") == 0) || (strcmp(coind->symbol,"WDC") == 0) || (strcmp(coind->symbol2, "WDC") == 0) || (strcmp(coind->symbol,"BCR") == 0) || (strcmp(coind->symbol2, "BCR") == 0) || (strcmp(coind->symbol,"WCN") == 0) || (strcmp(coind->symbol2, "WCN") == 0) || (strcmp(coind->symbol,"HUSTL") == 0) || (strcmp(coind->symbol2, "HUSTL") == 0) || (strcmp(coind->symbol,"DSV") == 0) || (strcmp(coind->symbol2, "DSV") == 0) || (strcmp(coind->symbol,"CATY") == 0) || (strcmp(coind->symbol2, "CATY") == 0) || (strcmp(coind->symbol,"CTC") == 0) || (strcmp(coind->symbol2, "CTC") == 0) || (strcmp(coind->symbol,"UQB") == 0) || (strcmp(coind->symbol2, "UQB") == 0) || (strcmp(coind->symbol,"FAKE") == 0) || (strcmp(coind->symbol2, "FAKE") == 0) || (strcmp(coind->symbol,"XMY") == 0) || (strcmp(coind->symbol2, "XMY") == 0) || (strcmp(coind->symbol,"SCHO") == 0) || (strcmp(coind->symbol2, "SCHO") == 0) || (strcmp(coind->symbol,"BCC") == 0) || (strcmp(coind->symbol2, "BCC") == 0) || (strcmp(coind->symbol,"BTC") == 0) || (strcmp(coind->symbol2, "BTC") == 0));	
	if(getaddressinfo)
		json = rpc_call(&coind->rpc, "getaddressinfo", params);
	else
		json = rpc_call(&coind->rpc, "validateaddress", params);
	if(!json) return false;

	json_value *json_result = json_get_object(json, "result");
	if(!json_result)
	{
		json_value_free(json);
		return false;
	}

	bool isvalid = getaddressinfo || json_get_bool(json_result, "isvalid");
	if(!isvalid) stratumlog("%s wallet %s is not valid.\n", coind->name, coind->wallet);

	bool ismine = json_get_bool(json_result, "ismine");
	if(!ismine) stratumlog("%s wallet %s is not mine.\n", coind->name, coind->wallet);
	else isvalid = ismine;

	const char *p = json_get_string(json_result, "pubkey");
	strcpy(coind->pubkey, p ? p : "");

	const char *acc = json_get_string(json_result, "account");
	if (acc) strcpy(coind->account, acc);

	if (!base58_decode(coind->wallet, coind->script_pubkey))
		stratumlog("Warning: unable to decode %s %s script pubkey\n", coind->symbol, coind->wallet);

	coind->p2sh_address = json_get_bool(json_result, "isscript");

	// if base58 decode fails
	if (!strlen(coind->script_pubkey)) {
		const char *pk = json_get_string(json_result, "scriptPubKey");
		if (pk && strlen(pk) > 10) {
			strcpy(coind->script_pubkey, &pk[6]);
			coind->script_pubkey[strlen(pk)-6-4] = '\0';
			stratumlog("%s %s extracted script pubkey is %s\n", coind->symbol, coind->wallet, coind->script_pubkey);
		} else {
			stratumlog("%s wallet addr '%s' seems incorrect!'", coind->symbol, coind->wallet);
		}
	}
	json_value_free(json);

	return isvalid && ismine;
}

void coind_init(YAAMP_COIND *coind)
{
	char params[YAAMP_SMALLBUFSIZE];
	char account[YAAMP_SMALLBUFSIZE];

	yaamp_create_mutex(&coind->mutex);

	strcpy(account, coind->account);
	if(!strcmp(coind->rpcencoding, "DCR")) {
		coind->usegetwork = true;
		//sprintf(account, "default");
	}

	bool valid = coind_validate_address(coind);
	if(valid) return;

	sprintf(params, "[\"%s\"]", account);

	json_value *json = rpc_call(&coind->rpc, "getaccountaddress", params);
	if(!json)
	{
		json = rpc_call(&coind->rpc, "getaddressesbyaccount", params);
		if (json && json_is_array(json) && json->u.object.length) {
			debuglog("is array...");
			if (json->u.object.values[0].value->type == json_string)
				json = json->u.object.values[0].value;
		}
		if (!json) {
			stratumlog("ERROR getaccountaddress %s\n", coind->name);
			return;
		}
	}

	    bool is_dgb = ((strcmp(coind->symbol,"DGB") == 0) || (strcmp(coind->symbol2, "DGB") == 0));

    if (is_dgb) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_wcn = ((strcmp(coind->symbol,"WCN") == 0) || (strcmp(coind->symbol2, "WCN") == 0));

    if (is_wcn) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }

	    bool is_bns = ((strcmp(coind->symbol,"BNS") == 0) || (strcmp(coind->symbol2, "BNS") == 0));

    if (is_bns) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_wdc = ((strcmp(coind->symbol,"WDC") == 0) || (strcmp(coind->symbol2, "WDC") == 0));

    if (is_wdc) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }

	    bool is_picoin = ((strcmp(coind->symbol,"PICOIN") == 0) || (strcmp(coind->symbol2, "PICOIN") == 0));

    if (is_picoin) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	    bool is_vvc = ((strcmp(coind->symbol,"VVC") == 0) || (strcmp(coind->symbol2, "VVC") == 0));

    if (is_vvc) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_pic = ((strcmp(coind->symbol,"PIC") == 0) || (strcmp(coind->symbol2, "PIC") == 0));

    if (is_pic) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	bool is_bcr = ((strcmp(coind->symbol,"BCR") == 0) || (strcmp(coind->symbol2, "BCR") == 0));

    if (is_bcr) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	bool is_btc = ((strcmp(coind->symbol,"BTC") == 0) || (strcmp(coind->symbol2, "BTC") == 0));

    if (is_btc) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_bcc = ((strcmp(coind->symbol,"BCC") == 0) || (strcmp(coind->symbol2, "BCC") == 0));

    if (is_bcc) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_xmy = ((strcmp(coind->symbol,"XMY") == 0) || (strcmp(coind->symbol2, "XMY") == 0));

    if (is_xmy) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }	

	    bool is_fake = ((strcmp(coind->symbol,"FAKE") == 0) || (strcmp(coind->symbol2, "FAKE") == 0));

    if (is_fake) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }	
	
	    bool is_dsv = ((strcmp(coind->symbol,"DSV") == 0) || (strcmp(coind->symbol2, "DSV") == 0));

    if (is_dsv) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }	

	    bool is_hustl = ((strcmp(coind->symbol,"HUSTL") == 0) || (strcmp(coind->symbol2, "HUSTL") == 0));

    if (is_hustl) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }	
	
	bool is_caty = ((strcmp(coind->symbol,"CATY") == 0) || (strcmp(coind->symbol2, "CATY") == 0));

    if (is_caty) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }	
	
	    bool is_uqb = ((strcmp(coind->symbol,"UQB") == 0) || (strcmp(coind->symbol2, "UQB") == 0));

    if (is_uqb) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_ctc = ((strcmp(coind->symbol,"CTC") == 0) || (strcmp(coind->symbol2, "CTC") == 0));

    if (is_ctc) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	    bool is_scho = ((strcmp(coind->symbol,"SCHO") == 0) || (strcmp(coind->symbol2, "SCHO") == 0));

    if (is_scho) {
        if (json) json_value_free(json);

        json = rpc_call(&coind->rpc, "getnewaddress", params);
    }
	
	if (json->u.object.values[0].value->type == json_string) {
		strcpy(coind->wallet, json->u.object.values[0].value->u.string.ptr);
	}
	else {
		strcpy(coind->wallet, "");
		stratumlog("ERROR getaccountaddress %s\n", coind->name);
	}

	json_value_free(json);

	coind_validate_address(coind);
	if (strlen(coind->wallet)) {
		debuglog(">>>>>>>>>>>>>>>>>>>> using wallet %s %s\n",
			coind->wallet, coind->account);
	}
}

///////////////////////////////////////////////////////////////////////////////

//void coind_signal(YAAMP_COIND *coind)
//{
//	debuglog("coind_signal %s\n", coind->symbol);
//	CommonLock(&coind->mutex);
//	pthread_cond_signal(&coind->cond);
//	CommonUnlock(&coind->mutex);
//}

void coind_terminate(YAAMP_COIND *coind)
{
	debuglog("disconnecting from coind %s\n", coind->symbol);

	rpc_close(&coind->rpc);
#ifdef HAVE_CURL
	if (coind->rpc.curl) rpc_curl_close(&coind->rpc);
#endif

	pthread_mutex_unlock(&coind->mutex);
	pthread_mutex_destroy(&coind->mutex);
//	pthread_cond_destroy(&coind->cond);

	object_delete(coind);

//	pthread_exit(NULL);
}

//void *coind_thread(void *p)
//{
//	YAAMP_COIND *coind = (YAAMP_COIND *)p;
//	debuglog("connecting to coind %s\n", coind->symbol);

//	bool b = rpc_connect(&coind->rpc);
//	if(!b) coind_terminate(coind);

//	coind_init(coind);

//	CommonLock(&coind->mutex);
//	while(!coind->deleted)
//	{
//		job_create_last(coind, true);
//		pthread_cond_wait(&coind->cond, &coind->mutex);
//	}

//	coind_terminate(coind);
//}






