/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include "usGetBundleContext.h"
#include "usBundleHooks_p.h"

#include "usBundle.h"
#include "usBundleEventHook.h"
#include "usBundleFindHook.h"
#include "usCoreBundleContext_p.h"
#include "usBundleContextPrivate.h"
#include "usServiceReferenceBasePrivate.h"

namespace us {

BundleHooks::BundleHooks(CoreBundleContext* ctx)
  : coreCtx(ctx)
{
}

Bundle BundleHooks::FilterBundle(
    const BundleContext& context,
    const Bundle& bundle
    ) const
{
  if(!bundle)
  {
    return bundle;
  }

  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<BundleFindHook>(), srl);
  if (srl.empty())
  {
    return bundle;
  }
  else
  {
    std::vector<Bundle> ml;
    ml.push_back(bundle);
    this->FilterBundles(context, ml);
    return ml.empty() ? Bundle() : bundle;
  }
}

void BundleHooks::FilterBundles(
    const BundleContext& context,
    std::vector<Bundle>& bundles
    ) const
{
  std::vector<ServiceRegistrationBase> srl;
  coreCtx->services.Get(us_service_interface_iid<BundleFindHook>(), srl);
  ShrinkableVector<Bundle> filtered(bundles);

  auto selfBundle = GetBundleContext().GetBundle();
  std::sort(srl.begin(), srl.end());
  for (auto srBaseIter = srl.rbegin(), srBaseEnd = srl.rend(); srBaseIter != srBaseEnd; ++srBaseIter)
  {
    ServiceReference<BundleFindHook> sr = srBaseIter->GetReference();
    std::shared_ptr<BundleFindHook> fh = std::static_pointer_cast<BundleFindHook>(sr.d.load()->GetService(GetPrivate(selfBundle).get()));
    if (fh)
    {
      try
      {
        fh->Find(context, filtered);
      }
      catch (const std::exception& e)
      {
        US_WARN << "Failed to call Bundle FindHook  #" << sr.GetProperty(Constants::SERVICE_ID).ToString()
                << ": " << e.what();
      }
      catch (...)
      {
        US_WARN << "Failed to call Bundle FindHook  #" << sr.GetProperty(Constants::SERVICE_ID).ToString()
                << ": unknown exception type";
      }
    }
  }
}

void BundleHooks::FilterBundleEventReceivers(const BundleEvent& evt,
                                             ServiceListeners::BundleListenerMap& bundleListeners)
{
  std::vector<ServiceRegistrationBase> eventHooks;
  coreCtx->services.Get(us_service_interface_iid<BundleEventHook>(), eventHooks);

  {
    auto l = coreCtx->listeners.bundleListenerMap.Lock(); US_UNUSED(l);
    bundleListeners = coreCtx->listeners.bundleListenerMap.value;
  }

  if(!eventHooks.empty())
  {
    std::vector<BundleContext> bundleContexts;
    for (auto& le : bundleListeners)
    {
      bundleContexts.push_back(MakeBundleContext(le.first->shared_from_this()));
    }
    std::sort(bundleContexts.begin(), bundleContexts.end());
    bundleContexts.erase(std::unique(bundleContexts.begin(), bundleContexts.end()), bundleContexts.end());

    const std::size_t unfilteredSize = bundleContexts.size();
    ShrinkableVector<BundleContext> filtered(bundleContexts);

    std::sort(eventHooks.begin(), eventHooks.end());
    for (auto iter = eventHooks.rbegin(), iterEnd = eventHooks.rend(); iter != iterEnd; ++iter)
    {
      ServiceReference<BundleEventHook> sr;
      try
      {
        sr = iter->GetReference();
      }
      catch (const std::logic_error& e)
      {
        US_WARN << "Failed to get event hook service reference: " << e.what();
        continue;
      }

      std::shared_ptr<BundleEventHook> eh = std::static_pointer_cast<BundleEventHook>(sr.d.load()->GetService(GetPrivate(GetBundleContext().GetBundle()).get()));
      if (eh)
      {
        try
        {
          eh->Event(evt, filtered);
        }
        catch (const std::exception& e)
        {
          US_WARN << "Failed to call Bundle EventHook #" << sr.GetProperty(Constants::SERVICE_ID).ToString()
                  << ": " << e.what();
        }
        catch (...)
        {
          US_WARN << "Failed to call Bundle EventHook #" << sr.GetProperty(Constants::SERVICE_ID).ToString()
                  << ": unknown exception type";
        }
      }
    }

    if (unfilteredSize != bundleContexts.size())
    {
      for (ServiceListeners::BundleListenerMap::iterator le = bundleListeners.begin();
           le != bundleListeners.end();)
      {
        if(std::find_if(bundleContexts.begin(), bundleContexts.end(), [&le](const BundleContext& bc) { return GetPrivate(bc) == le->first;}) == bundleContexts.end())
        {
          bundleListeners.erase(le++);
        }
        else
        {
          ++le;
        }
      }
    }
  }
}

}