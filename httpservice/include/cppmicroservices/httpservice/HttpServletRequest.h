/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef CPPMICROSERVICES_HTTPSERVLETREQUEST_H
#define CPPMICROSERVICES_HTTPSERVLETREQUEST_H

#include "cppmicroservices/httpservice/HttpServiceExport.h"
#include "cppmicroservices/Any.h"
#include "cppmicroservices/SharedData.h"

#include <string>
#include <vector>

namespace cppmicroservices {

class ServletContext;
struct HttpServletRequestPrivate;
struct HttpServletPartPrivate;
class HttpServletPart;

class US_HttpService_EXPORT HttpServletRequest
{
public:

  ~HttpServletRequest();

  HttpServletRequest(const HttpServletRequest& o);
  HttpServletRequest& operator=(const HttpServletRequest& o);

  std::shared_ptr<ServletContext> GetServletContext() const;

  Any GetAttribute(const std::string& name) const;

  std::vector<std::string> GetAttributeNames() const;

  std::size_t GetContentLength() const;

  std::string GetContentType() const;

  std::string GetLocalName() const;

  std::string GetRemoteHost() const;

  int GetLocalPort() const;

  int GetRemotePort() const;

  std::string GetScheme() const;

  std::string GetServerName() const;

  int GetServerPort() const;

  std::string GetProtocol() const;

  std::string GetContextPath() const;

  std::string GetPathInfo() const;

  std::string GetRequestUri() const;

  std::string GetRequestUrl() const;

  std::string GetServletPath() const;

  std::string GetQueryString() const;

  std::string GetHeader(const std::string& name) const;

  long long GetDateHeader(const std::string& name) const;

  std::vector<std::string> GetHeaderNames() const;

  std::vector<std::string> GetHeaders(const std::string& name) const;

  std::string GetMethod() const;

  std::vector<std::pair<std::string, float> > GetAcceptHeader() const;

  void RemoveAttribute(const std::string& name);

  void SetAttribute(const std::string& name, const Any& value);

  /*!
   * Return the value of asked query parameter or empty Any if the parameter is
   * not found
   */
  Any GetParameter(const std::string& name) const;

  /*!
   *
   * Gets all the Part components of this request, provided that it is of type
   * multipart/form-data.
   */
  std::vector<HttpServletPart*> GetParts() const;

  /*!
   * Gets the Part with the given name.
   */
  HttpServletPart* GetPart(const std::string& name) const;

  /*!
   * Triggers the load of files transmitted by HTTP POST, multipart/form-data
   * encoded
   *
   * An optional callback can be used to perform special operation on the
   * ServletPart
   */
  void ReadParts(void* callback(HttpServletPart*) = 0);

  HttpServletRequest(HttpServletRequestPrivate* d);

private:

  friend class ServletHandler;

  ExplicitlySharedDataPointer<HttpServletRequestPrivate> d;

  // CiveWeb callbacks used when calling ReadParts()

  static int field_found(const char* key,
                         const char* filename,
                         char* path,
                         size_t pathlen,
                         void* user_data);
  static int field_store(const char* path,
                         long long file_size,
                         void* user_data);
  static int field_get(const char* key,
                       const char* value,
                       size_t valuelen,
                       void* user_data);
};

}

#endif // CPPMICROSERVICES_HTTPREQUEST_H
