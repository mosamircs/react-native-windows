// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"

#include "ButtonViewManager.h"

#include <Utils/ValueUtils.h>
#include <Views/ShadowNodeBase.h>

#include <IReactInstance.h>

#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

namespace react {
namespace uwp {
namespace polyester {

class ButtonShadowNode : public ContentControlShadowNode {
  using Super = ContentControlShadowNode;

 public:
  ButtonShadowNode() = default;
  void createView() override;

 private:
  winrt::Button::Click_revoker m_buttonClickRevoker{};
};

void ButtonShadowNode::createView() {
  Super::createView();

  auto button = GetView().as<winrt::Button>();

  m_buttonClickRevoker = button.Click(winrt::auto_revoke, [=](auto &&, auto &&) {
    auto instance = GetViewManager()->GetReactInstance().lock();
    folly::dynamic eventData = folly::dynamic::object("target", m_tag);
    if (instance != nullptr)
      instance->DispatchEvent(m_tag, "topClick", std::move(eventData));
  });
}

ButtonViewManager::ButtonViewManager(const std::shared_ptr<IReactInstance> &reactInstance)
    : ContentControlViewManager(reactInstance) {}

const char *ButtonViewManager::GetName() const {
  // TODO: Is this right? Or should it be RCTButton?
  return "PLYButton";
}

folly::dynamic ButtonViewManager::GetNativeProps() const {
  auto props = Super::GetNativeProps();

  props.update(folly::dynamic::object("accessibilityLabel", "string")("disabled", "boolean")("buttonType", "string"));

  return props;
}

folly::dynamic ButtonViewManager::GetExportedCustomDirectEventTypeConstants() const {
  auto directEvents = Super::GetExportedCustomDirectEventTypeConstants();
  directEvents["topClick"] = folly::dynamic::object("registrationName", "onClick");

  return directEvents;
}

facebook::react::ShadowNode *ButtonViewManager::createShadow() const {
  return new ButtonShadowNode();
}

XamlView ButtonViewManager::CreateViewCore(int64_t /*tag*/) {
  winrt::Button button = winrt::Button();
  return button;
}

void ButtonViewManager::UpdateProperties(ShadowNodeBase *nodeToUpdate, const folly::dynamic &reactDiffMap) {
  auto button = nodeToUpdate->GetView().as<winrt::Button>();
  if (button == nullptr)
    return;

  for (const auto &pair : reactDiffMap.items()) {
    const std::string &propertyName = pair.first.getString();
    const folly::dynamic &propertyValue = pair.second;

    if (propertyName == "disabled") {
      if (propertyValue.isBool())
        button.IsEnabled(!propertyValue.asBool());
    }

    continue;
  }

  Super::UpdateProperties(nodeToUpdate, reactDiffMap);
}
} // namespace polyester
} // namespace uwp
} // namespace react
