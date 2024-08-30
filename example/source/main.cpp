#define TESLA_INIT_IMPL   // If you have more than one file using the tesla header, only define this in the main one
#include <nikola.hpp>     // The Nikola Header
#include <memory>


class GuiSecondary : public tsl::Gui {
public:
    GuiSecondary() {}

    tsl::elm::Element* createUI() override
  {
    auto* rootFrame = new tsl::elm::OverlayFrame(
        "Nikola Example", "v1.4.0 - Secondary Gui");
    rootFrame->setContent(new tsl::elm::DebugRectangle(
        tsl::gfx::Color {0x8, 0x3, 0x8, 0xF}));
    return rootFrame;
  }
};

class GuiTest : public tsl::Gui {
public:
    GuiTest(u8 arg1, u8 arg2, bool arg3) { }

    // Called when this Gui gets loaded to create the UI
    // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
    tsl::elm::Element* createUI() override {
        // A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
        // If you need more information in the header or want to change it look, use a HeaderOverlayFrame.
        auto frame = new tsl::elm::OverlayFrame("Nikola Example", "v1.3.2");

        // A list that can contain sub elements and handles scrolling
        auto list = new tsl::elm::List();


        auto *clickableListItem = new tsl::elm::ListItem("Clickable List Item");
        clickableListItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiSecondary>();
                return true;
            }

            return false;
        });

        list->addItem(clickableListItem);
        list->addItem(new tsl::elm::ListItem("Default List Item"));
        list->addItem(new tsl::elm::ListItem("Default List Item with an extra long name to trigger truncation and scrolling"));
        list->addItem(new tsl::elm::ToggleListItem("Toggle List Item", true));

        // Add the list to the frame for it to be drawn
        frame->setContent(list);

        // Return the frame to have it become the top level element of this Gui
        return frame;
    }

    // Called once every frame to update values
    virtual void update() override {

    }

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        return false;   // Return true here to signal the inputs have been consumed
    }
};


class OverlayTest : public tsl::Overlay {
public:
                                      // tesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
    void initServices() override {}   // Called at the start to initialize all services necessary for this Overlay
    void exitServices() override {}   // Called at the end to clean up all services previously initialized

    void onShow() override {}         // Called before overlay wants to change from invisible to visible state
    void onHide() override {}         // Called before overlay wants to change from visible to invisible state



    std::unique_ptr<tsl::Gui> loadInitialGui() override
    {
        return initially<GuiTest>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it constructor like this
    }
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}
