/*
  Copyright (C) 2017 National Institute For Space Research (INPE) - Brazil.

  This file is part of Earth Observation Web Services (EOWS).

  EOWS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3 as
  published by the Free Software Foundation.

  EOWS is distributed  "AS-IS" in the hope that it will be useful,
  but WITHOUT ANY WARRANTY OF ANY KIND; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with EOWS. See LICENSE. If not, write to
  e-sensing team at <esensing-team@dpi.inpe.br>.
 */

// EOWS
#include "wtscs.hpp"
#include "request.hpp"
#include "../core/http_response.hpp"
#include "../core/http_request.hpp"
#include "../core/logger.hpp"
#include "../core/service_operations_manager.hpp"
#include "../core/utils.hpp"
#include "../geoarray/data_types.hpp"
#include "../geoarray/geoarray_manager.hpp"
//#include "../exception.hpp"

//// C++ Standard Library
//#include <memory>


// Boost
#include <boost/format.hpp>

// RapidJSON
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cstdlib>

using namespace std;

string scidb_shared_directory;
string config_directory;
string name_service;

eows::wtscs::request* pRequestList;
static void return_exception(const char* msg, eows::core::http_response& res);

void eows::wtscs::status_handler::do_get(const eows::core::http_request& req, eows::core::http_response& res)
{
  // TODO: See how to implement the status_handler methods!.

  string return_msg("\"status_handler\"");

  res.set_status(eows::core::http_response::OK);

  res.add_header(eows::core::http_response::CONTENT_TYPE, "text/plain; charset=utf-8");
  res.add_header(eows::core::http_response::ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  res.write(return_msg.c_str(), return_msg.size());
}

void eows::wtscs::list_algorithms_handler::do_get(const eows::core::http_request& req, eows::core::http_response& res)
{
  vector<string> algorithms{ "twdtw", "bfast", "bfast-monitor" };

  rapidjson::StringBuffer buff;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buff);

  writer.StartObject();

  writer.Key("algorithms", static_cast<rapidjson::SizeType>(sizeof("algorithms") -1));
  eows::core::write_string_array(algorithms.begin(), algorithms.end(), writer);

  writer.EndObject();

  res.set_status(eows::core::http_response::OK);

  res.add_header(eows::core::http_response::CONTENT_TYPE, "application/json; charset=utf-8");
  res.add_header(eows::core::http_response::ACCESS_CONTROL_ALLOW_ORIGIN, "*");

  res.write(buff.GetString(), buff.GetSize());
}

void eows::wtscs::classify_handler::do_post(const eows::core::http_request& req,
                                            eows::core::http_response& res)
{

  // Parsing the request into a Document.
  request* oRequest = new request;

  try
  {
    oRequest->set_UUID(name_service);
    oRequest->set_parameters(req.content(), scidb_shared_directory);

    oRequest->check_parameters();
    if((oRequest->get_status()).compare("Scheduled") == 0)
    {
      eows::wtscs::request* pTempPosition = pRequestList;
      eows::wtscs::request* pPrevPosition = pTempPosition;
      pTempPosition = pTempPosition->pNext;
      while(pTempPosition)
      {
        pPrevPosition = pTempPosition;
        pTempPosition = pTempPosition->pNext;
      }
      pPrevPosition->pNext = oRequest;

      // TODO: Save the request list on disk (wtscs_request_list.json)
      // You will use the config_directory string

      // Write afl
      string afl = oRequest->write_afl(dynamic_cast<eows::wtscs::twdtw_input_parameters*>(oRequest->input_parameters.get()));


      cout << afl;
      cout << endl;
      // system(afl);

      // It Sends the AFL request.
      // The answer is a URL wrapping the request UUID identifier.
      // Ex. http://localhost:7654/wtscs/status?UUID=123456687


      // assembly the response
      rapidjson::StringBuffer buff;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buff);

      writer.StartObject();

      writer.Key("UUID", static_cast<rapidjson::SizeType>(sizeof("UUID") -1));
      writer.String((oRequest->get_UUID()).c_str(), static_cast<rapidjson::SizeType>((oRequest->get_UUID()).length()));
      writer.EndObject();

      res.set_status(eows::core::http_response::OK);

      res.add_header(eows::core::http_response::CONTENT_TYPE, "application/json; charset=utf-8");
      res.add_header(eows::core::http_response::ACCESS_CONTROL_ALLOW_ORIGIN, "*");

      res.write(buff.GetString(), buff.GetSize());
    }
  }
  catch(const exception& e)
  {
    return_exception(e.what(), res);
  }
  catch(...)
  {
    return_exception("Unexpected error in WTSCS classify operation.", res);
  }
}

void eows::wtscs::open_request_list()
{
  scidb_shared_directory = "/home/edullapa/mydevel/eows/scidb/";
  config_directory = "/home/edullapa/mydevel/eows/codebase/share/eows/config/";
  name_service = "WTSCS";

  pRequestList = new request;


  EOWS_LOG_INFO("request file was analized!");
}

void eows::wtscs::initialize()
{
  EOWS_LOG_INFO("Initializing WTSCS...");

  //Read UUID using WTSCS
  eows::wtscs::open_request_list();

  unique_ptr<eows::wtscs::status_handler> s_h(new eows::wtscs::status_handler);
  eows::core::service_operations_manager::instance().insert("/wtscs/status", move(s_h));

  unique_ptr<list_algorithms_handler> la_h(new list_algorithms_handler);
  eows::core::service_operations_manager::instance().insert("/wtscs/list_algorithms", move(la_h));

  unique_ptr<classify_handler> c_h(new classify_handler);
  eows::core::service_operations_manager::instance().insert("/wtscs/classify", move(c_h));

  EOWS_LOG_INFO("WTSCS service initialized!");
}

void return_exception(const char* msg, eows::core::http_response& res)
{
  rapidjson::StringBuffer buff;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buff);

  writer.StartObject();

  writer.Key("exception", static_cast<rapidjson::SizeType>(sizeof("exception") -1));
  writer.String(msg);

  writer.EndObject();

  res.set_status(eows::core::http_response::OK);

  res.add_header(eows::core::http_response::CONTENT_TYPE, "application/json; charset=utf-8");
  res.add_header(eows::core::http_response::ACCESS_CONTROL_ALLOW_ORIGIN, "*");

  res.write(buff.GetString(), buff.GetSize());
}
