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

struct ShadowData
{
	int timestamp;
	int value;
};

afb::event data_event;

json_object *json_of_data(const afb::data &d)
{
	json_object *r = nullptr;
	afb::data j(afb::type_json_c(), d);
	if (j)
	{
		r = reinterpret_cast<json_object *>(const_cast<void *>(*j));
		r = json_object_get(r);
	}
	return r;
}

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

	struct ShadowData data;

	afb::received_data params(n, a);
	r = json_of_data(params[0]);

	json_object_object_get_ex(r, "timestamp", &jtimestamp);
	json_object_object_get_ex(r, "value", &jvalue);

	data.timestamp = json_object_get_int(jtimestamp);
	data.value = json_object_get_int(jvalue);

	if (data.timestamp % 5 == 0)
	{
		data.value = data.value + (std::rand() % 100);
	}
	json_object *obj;

	// create event data_csv
	wrap_json_pack(&obj, "{si si}",
				   "timestamp", data.timestamp,
				   "value", data.value);

	afb::dataset<1> dataset;
	dataset[0] = json_to_req_data(NULL, obj);
	data_event.push(dataset);
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

const afb_binding_t afbBindingExport = afb::binding("shadow", verbs, "Shadow App for PoC", mainctl);
