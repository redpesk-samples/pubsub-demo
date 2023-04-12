/*
 * Copyright (C) 2015-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 * Author: Clément Bénier <jose.bollo@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <mutex>
#include <map>
#include <set>

#include <string.h>
#include <json-c/json.h>
#include <wrap-json.h>

#define AFB_BINDING_VERSION 4
#include <afb/afb-binding>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <json-c/json.h>

struct ShadowData
{
	int timestamp;
	int value;
};

std::vector<ShadowData> data_shadow;
std::vector<ShadowData> data_release;

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

json_object *json_of_data(std::pair<unsigned, afb_data_t const *> dataset, unsigned index)
{
	return index < dataset.first ? json_of_data(dataset.second[index]) : nullptr;
}

static std::mutex mutex;

/**************************************************************************/

static struct ShadowData dispatch(unsigned n, afb_data_t const a[])
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
	return data;
}

static void dispatch_release(void *closure, const char *event, unsigned n, afb_data_t const a[], afb_api_t api)
{
	AFB_NOTICE("received released");
	data_release.push_back(dispatch(n, a));
}

static void dispatch_shadow(void *closure, const char *event, unsigned n, afb_data_t const a[], afb_api_t api)
{
	AFB_NOTICE("received shadow");
	const std::lock_guard<std::mutex> lock(mutex);
	data_shadow.push_back(dispatch(n, a));

	// simple compare for now: should received same values/same orders
	while (data_shadow.size() > 0 || data_shadow.size() > 0)
	{
		int s_timestamp = data_shadow[0].timestamp;
		int s_value = data_shadow[0].value;

		int r_timestamp = data_release[0].timestamp;
		int r_value = data_release[0].value;

		if (s_timestamp == r_timestamp && s_value == r_value)
		{
			AFB_NOTICE("same value %d %d", s_timestamp, s_value);
		}
		else
		{
			AFB_NOTICE("value differs (shadow=%d %d) != (release=%d %d)", s_timestamp, s_value, r_timestamp, r_value);
		}
		data_shadow.erase(data_shadow.begin());
		data_release.erase(data_release.begin());
	}
}

int mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
	afb::api a(api);

	if (ctlid == afb_ctlid_Init)
	{
		AFB_NOTICE("init");
		afb_api_require_api(api, "release", 0);
		afb_api_require_api(api, "shadow", 0);
		afb_api_call_sync(api, "shadow", "subscribe", NULL, NULL, NULL, NULL, NULL);
		afb_api_call_sync(api, "release", "subscribe", NULL, NULL, NULL, NULL, NULL);
		afb_api_event_handler_add(api, "shadow/data_event", dispatch_shadow, NULL);
		afb_api_event_handler_add(api, "release/data_event", dispatch_release, NULL);
	}
	return 0;
}

const afb_verb_t verbs[] = {
	afb::verbend()};

const afb_binding_t afbBindingExport = afb::binding("compare", verbs, "compare", mainctl);
