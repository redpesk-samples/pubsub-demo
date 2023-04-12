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
afb_timer_t timer;

// Structure to hold time and value data
struct Data
{
	int time;
	int value;
};

std::vector<Data> data_csv;
int data_csv_len = 0;
int current_data_csv = 0;

// Function to read CSV file
std::vector<Data> readCSV(const std::string &filename)
{
	std::vector<Data> data;
	std::ifstream file(filename);
	if (file.is_open())
	{
		std::string line;
		// Read each line from CSV file
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string timeStr, valueStr;
			int time, value;
			// Parse time and value from CSV line
			if (std::getline(iss, timeStr, ';') && std::getline(iss, valueStr, ';'))
			{
				// Create Data struct and add to vector
				Data d;
				d.time = std::stoi(timeStr);
				d.value = std::stoi(valueStr);
				data.push_back(d);
			}
		}
		file.close();
	}
	else
	{
		AFB_ERROR("Failed to open file CSV");
	}
	return data;
}

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

afb::data json_to_req_data(afb::req req, json_object *obj)
{
	afb::data r(afb::type_json_c(), obj, 0,
				reinterpret_cast<void (*)(void *)>(json_object_put),
				reinterpret_cast<void *>(obj));
	return r;
}

void reply_error(afb::req req, const char *text)
{
	afb::dataset<1> a;
	a[0] = json_to_req_data(req, json_object_new_string(text));
	req.reply(-1, a);
}

void send_data(void)
{
	// while (1)
	// {
	// 	afb::dataset<1> a;
	// 	a[0] = "ok";
	// 	data_event.push(a);
	// 	TimeEv
	// }
}

void timed_event(afb_timer_t timer, void *closure, int decount)
{
	afb::dataset<1> a;

	Data da = data_csv.at(current_data_csv);
	current_data_csv++;
	if (current_data_csv >= data_csv_len)
	{
		current_data_csv = 0;
	}

	json_object *obj;

	// create event data_csv
	wrap_json_pack(&obj, "{si si}",
				   "timestamp", da.time,
				   "value", da.value);

	a[0] = json_to_req_data(NULL, obj);
	data_event.push(a);
}

void subscribe(afb::req req, afb::received_data params)
{
	json_object *args, *val;
	AFB_INFO("Subscribing");
	req.subscribe(data_event);
	req.reply();

	data_csv = readCSV("fic.csv");
	data_csv_len = data_csv.size();
	if (data_csv_len == 0)
	{
		return;
	}

	// first = std::thread(send_data)

	afb_timer_create(&timer,
					 /*start:*/ 0 /*relative*/, 1 /*sec*/, 0 /*msec*/,
					 /*occur:*/ 0 /*infinite*/, 1000 /*period msec*/, 10 /*accuracy msec*/,
					 /*action:*/ timed_event, data_event, 0 /*no unref*/);

	// afb::dataset<1> a;
	// a[0] = json_to_req_data(req, json_object_new_string("ok"));
	// data_event.push(a);
}

void unsubscribe(afb::req req, afb::received_data params)
{
	afb_timer_unref(timer);
	AFB_ERROR("ok");
	req.unsubscribe(data_event);
	req.reply();
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
	}
	return 0;
}

const afb_verb_t verbs[] = {
	afb::verb<subscribe>("subscribe"),
	afb::verb<unsubscribe>("unsubscribe"),
	afb::verbend()};

const afb_binding_t afbBindingExport = afb::binding("sender-app", verbs, "Sender App for PoC", mainctl);
