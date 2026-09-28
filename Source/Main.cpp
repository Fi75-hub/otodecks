#include <JuceHeader.h>
#include "MainComponent.h"

// OtoDecksApplication class
class OtoDecksApplication  : public JUCEApplication // Inherits from JUCEApplication
{
public:
    OtoDecksApplication() {} // Constructor
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    // Initialise the application
    void initialise (const String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    // Shutdown the application
    void shutdown() override
    {
        mainWindow = nullptr;
    }

    // System requested quit
    void systemRequestedQuit() override
    {
        quit();
    }

    // Another instance started
    void anotherInstanceStarted (const String& commandLine) override
    {
    }

    // MainWindow class
    class MainWindow    : public DocumentWindow,
                          public KeyListener
    {
    public:
        // Constructor
        MainWindow (String name)  : DocumentWindow (name,
                                                    Colour(30, 30, 35),
                                                    DocumentWindow::minimiseButton | 
                                                    DocumentWindow::maximiseButton | 
                                                    DocumentWindow::closeButton)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            // Enable resizing
            setResizable (true, true);
            
            // Set size limits
            setResizeLimits(800, 600, 3840, 2160);
            
            auto displayArea = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            
            // Set default size
            int defaultWidth = jmin(1200, (int)(displayArea.getWidth() * 0.8f));
            int defaultHeight = jmin(800, (int)(displayArea.getHeight() * 0.8f));
            
            // Set size limits
            defaultWidth = jmax(800, defaultWidth);
            defaultHeight = jmax(600, defaultHeight);
            
            // Centre the window
            centreWithSize(defaultWidth, defaultHeight);
           #endif

            setVisible (true);
            
            // Keyboard listener for F11 fullscreen
            getContentComponent()->addKeyListener(this);
            getContentComponent()->setWantsKeyboardFocus(true);
        }
        
        // Destructor
        ~MainWindow() override
        {
            if (getContentComponent() != nullptr)
                getContentComponent()->removeKeyListener(this);
        }
        
        // Key pressed
        bool keyPressed(const KeyPress& key, Component* originatingComponent) override
        {
            // F11 toggle fullscreen
            if (key == KeyPress::F11Key)
            {
                setFullScreen(!isFullScreen());
                return true;
            }
            
            // Escape exit fullscreen
            if (key == KeyPress::escapeKey && isFullScreen())
            {
                setFullScreen(false);
                return true;
            }
            
            return false;
        }

        // Close button pressed
        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// Entry point for the application
START_JUCE_APPLICATION (OtoDecksApplication)