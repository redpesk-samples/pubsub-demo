// Copyright (C) 2015-2023 IoT.bzh Company
// Author: José Bollo <jose.bollo@iot.bzh>
//
// SPDX-License-Identifier: LGPL-3.0-only

#include <string>
#include <cstring>
#include <utility>

#include <json-c/json.h>
#include "wrap-json.h"

#define AFB_BINDING_VERSION 4
#include <afb/afb-binding>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

afb::event data_event;

afb::data json_to_req_data(afb::req req, json_object *obj)
{
	afb::data r(afb::type_json_c(), obj, 0,
				reinterpret_cast<void (*)(void *)>(json_object_put),
				reinterpret_cast<void *>(obj));
	return r;
}

void subscribe(afb::req req, afb::received_data params)
{
	json_object *args, *val;
	AFB_INFO("Subscribing");

	req.subscribe(data_event);
	req.reply();
}

void unsubscribe(afb::req req, afb::received_data params)
{
	AFB_NOTICE("Unsubscribe");
	req.unsubscribe(data_event);
	req.reply();
}

void dispatch(void *closure, const char *event, unsigned n, afb_data_t const a[], afb_api_t api)
{
	struct json_object *r;
	struct json_object *jtimestamp, *jvalue;

	afb::received_data params(n, a);
	r = json_of_data(params[0]);

	afb::dataset<1> a;
	a[0] = json_to_req_data(NULL, r);
	data_event.push(a);
}

int mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
	afb::api a(api);

	if (ctlid == afb_ctlid_Init)
	{
		AFB_NOTICE("init");
		data_event = a.new_event("data_event");
		if (!data_event)
		{
			AFB_ERROR("Can't create events");
			return -1;
		}

		afb_api_require_api(api, "reader", 1);
		afb_api_call_sync(api, "reader", "subscribe", NULL, NULL, NULL, NULL, NULL);
		afb_api_event_handler_add(api, "reader/data_event", dispatch, NULL);
	}
	return 0;
}

const afb_verb_t verbs[] = {
	afb::verb<subscribe>("subscribe"),
	afb::verb<unsubscribe>("unsubscribe"),
	afb::verbend()};

const afb_binding_t afbBindingExport = afb::binding("release", verbs, "Release App for PoC", mainctl);
