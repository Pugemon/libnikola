//
// Created by pugemon on 30.08.24.
//
#include "nikola/tesla.hpp"
#include "nikola/tesla/impl.hpp"

namespace tsl
{

#pragma region class_GUI

Gui::~Gui()
{
  if (this->m_topElement != nullptr)
    delete this->m_topElement;
}

bool Gui::handleInput(u64 keysDown,
                      u64 keysHeld,
                      const HidTouchState& touchPos,
                      HidAnalogStickState leftJoyStick,
                      HidAnalogStickState rightJoyStick)
{
  return false;
}

elm::Element* Gui::getTopElement()
{
  return this->m_topElement;
}

elm::Element* Gui::getFocusedElement()
{
  return this->m_focusedElement;
}

void Gui::requestFocus(elm::Element* element, FocusDirection direction)
{
  elm::Element* oldFocus = this->m_focusedElement;

  if (element != nullptr) {
    this->m_focusedElement = element->requestFocus(oldFocus, direction);

    if (oldFocus != nullptr)
      oldFocus->setFocused(false);

    if (this->m_focusedElement != nullptr) {
      this->m_focusedElement->setFocused(true);
    }
  }

  if (oldFocus == this->m_focusedElement && this->m_focusedElement != nullptr)
    this->m_focusedElement->shakeHighlight(direction);
}

void Gui::removeFocus(elm::Element* element)
{
  if (element == nullptr || element == this->m_focusedElement)
    this->m_focusedElement = nullptr;
}

void Gui::draw(gfx::Renderer* renderer)
{
  if (this->m_topElement != nullptr)
    this->m_topElement->draw(renderer);
}

#pragma endregion class_GUI

#pragma region class_Overlay

std::unique_ptr<tsl::Gui>& Overlay::getCurrentGui()
{
  return this->m_guiStack.top();
}

void Overlay::show()
{
  if (this->m_disableNextAnimation) {
    this->m_animationCounter = 5;
    this->m_disableNextAnimation = false;
  } else {
    this->m_fadeInAnimationPlaying = false;
    this->m_animationCounter = 0;
  }

  this->onShow();
}

void Overlay::hide()
{
  if (this->m_disableNextAnimation) {
    this->m_animationCounter = 0;
    this->m_disableNextAnimation = false;
  } else {
    this->m_fadeOutAnimationPlaying = false;
    this->m_animationCounter = 5;
  }

  this->onHide();
}

bool Overlay::fadeAnimationPlaying()
{
  return this->m_fadeInAnimationPlaying || this->m_fadeOutAnimationPlaying;
}

void Overlay::close()
{
  this->m_shouldClose = true;
}

Overlay* const Overlay::get()
{
  return Overlay::s_overlayInstance;
}

void Overlay::initScreen()
{
  gfx::Renderer::get().init();
}

void Overlay::exitScreen()
{
  gfx::Renderer::get().exit();
}

bool Overlay::shouldHide()
{
  return this->m_shouldHide;
}

bool Overlay::shouldClose()
{
  return this->m_shouldClose;
}

void Overlay::animationLoop()
{
  if (this->m_fadeInAnimationPlaying) {
    this->m_animationCounter++;

    if (this->m_animationCounter >= 5)
      this->m_fadeInAnimationPlaying = false;
  }

  if (this->m_fadeOutAnimationPlaying) {
    this->m_animationCounter--;

    if (this->m_animationCounter == 0) {
      this->m_fadeOutAnimationPlaying = false;
      this->m_shouldHide = true;
    }
  }

  gfx::Renderer::setOpacity(0.2 * this->m_animationCounter);
}

void Overlay::loop()
{
  auto& renderer = gfx::Renderer::get();

  renderer.startFrame();

  this->animationLoop();
  this->getCurrentGui()->update();
  this->getCurrentGui()->draw(&renderer);

  renderer.endFrame();
}

void Overlay::handleInput(u64 keysDown,
                          u64 keysHeld,
                          const HidTouchState touchPos,
                          HidAnalogStickState joyStickPosLeft,
                          HidAnalogStickState joyStickPosRight)
{
  auto& currentGui = this->getCurrentGui();
  auto currentFocus = currentGui->getFocusedElement();

  if (currentFocus == nullptr) {
    if (elm::Element* topElement = currentGui->getTopElement();
        topElement == nullptr)
    {
      return;
    } else
      currentFocus = topElement;
  }

  bool handled = false;
  elm::Element* parentElement = currentFocus;
  do {
    handled = parentElement->onClick(keysDown);
    parentElement = parentElement->getParent();
  } while (!handled && parentElement != nullptr);

  if (currentGui != this->getCurrentGui())
    return;

  handled = handled
      | currentGui->handleInput(
          keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);

  if (!handled) {
    if (keysDown & KEY_UP)
      currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Up);
    else if (keysDown & KEY_DOWN)
      currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Down);
    else if (keysDown & KEY_LEFT)
      currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Left);
    else if (keysDown & KEY_RIGHT)
      currentGui->requestFocus(currentFocus->getParent(),
                               FocusDirection::Right);
  }
}

void Overlay::clearScreen()
{
  auto& renderer = gfx::Renderer::get();

  renderer.startFrame();
  renderer.clearScreen();
  renderer.endFrame();
}

void Overlay::resetFlags()
{
  this->m_shouldHide = false;
  this->m_shouldClose = false;
}

void Overlay::disableNextAnimation()
{
  this->m_disableNextAnimation = true;
}

std::unique_ptr<tsl::Gui>& Overlay::changeTo(std::unique_ptr<tsl::Gui>&& gui)
{
  gui->m_topElement = gui->createUI();
  gui->requestFocus(gui->m_topElement, FocusDirection::None);

  this->m_guiStack.push(std::move(gui));

  return this->m_guiStack.top();
}

void Overlay::goBack()
{
  if (!this->m_closeOnExit && this->m_guiStack.size() == 1) {
    this->hide();
    return;
  }

  if (!this->m_guiStack.empty())
    this->m_guiStack.pop();

  if (this->m_guiStack.empty())
    this->close();
}

#pragma endregion class_Overlay


void goBack()
{
  Overlay::get()->goBack();
}

void setNextOverlay(std::string ovlPath, std::string args)
{
  args += " --skipCombo";

  envSetNextLoad(ovlPath.c_str(), args.c_str());
}

template<impl::LaunchFlags launchFlags>
int loop(OverlayFactory overlay, int argc, char** argv)
{

  impl::SharedThreadData shData;

  shData.running = true;

  Thread hidPollerThread, homeButtonDetectorThread, powerButtonDetectorThread;
  threadCreate(&hidPollerThread,
               impl::hidInputPoller<launchFlags>,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadCreate(&homeButtonDetectorThread,
               impl::homeButtonDetector,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadCreate(&powerButtonDetectorThread,
               impl::powerButtonDetector,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadStart(&hidPollerThread);
  threadStart(&homeButtonDetectorThread);
  threadStart(&powerButtonDetectorThread);

  eventCreate(&shData.comboEvent, false);

  auto overlayFactory = overlay();
  auto& overlayInstance = tsl::Overlay::s_overlayInstance;
  overlayInstance = overlayFactory.release();
  overlayInstance->m_closeOnExit =
      (u8(launchFlags) & u8(impl::LaunchFlags::CloseOnExit))
      == u8(impl::LaunchFlags::CloseOnExit);

  tsl::hlp::doWithSmSession([&overlayInstance] { overlayInstance->initServices(); });
  overlayInstance->initScreen();
  overlayInstance->changeTo(overlayInstance->loadInitialGui());

  // Argument parsing
  for (u8 arg = 0; arg < argc; arg++) {
    if (strcasecmp(argv[arg], "--skipCombo") == 0) {
      eventFire(&shData.comboEvent);
      overlayInstance->disableNextAnimation();
    }
  }

  while (shData.running) {
    eventWait(&shData.comboEvent, UINT64_MAX);
    eventClear(&shData.comboEvent);
    shData.overlayOpen = true;

    hlp::requestForeground(true);

    overlayInstance->show();
    overlayInstance->clearScreen();

    while (shData.running) {
      overlayInstance->loop();

      {
        std::scoped_lock lock(shData.dataMutex);
        if (!overlayInstance->fadeAnimationPlaying()) {
          overlayInstance->handleInput(shData.keysDownPending,
                               shData.keysHeld,
                               shData.touchState.touches[0],
                               shData.joyStickPosLeft,
                               shData.joyStickPosRight);
        }
        shData.keysDownPending = 0;
      }

      if (overlayInstance->shouldHide())
        break;

      if (overlayInstance->shouldClose())
        shData.running = false;
    }

    overlayInstance->clearScreen();
    overlayInstance->resetFlags();

    hlp::requestForeground(false);

    shData.overlayOpen = false;
    eventClear(&shData.comboEvent);
  }

  eventClose(&shData.homeButtonPressEvent);
  eventClose(&shData.powerButtonPressEvent);
  eventClose(&shData.comboEvent);

  threadWaitForExit(&hidPollerThread);
  threadClose(&hidPollerThread);
  threadWaitForExit(&homeButtonDetectorThread);
  threadClose(&homeButtonDetectorThread);
  threadWaitForExit(&powerButtonDetectorThread);
  threadClose(&powerButtonDetectorThread);

  overlayInstance->exitScreen();
  overlayInstance->exitServices();

  delete overlayInstance;

  return 0;
}

}  // namespace nikola::tsl