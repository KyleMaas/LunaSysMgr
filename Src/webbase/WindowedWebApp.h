/**
 * @file
 * 
 * Base UI functionality for app container classes to derive from.
 *
 * @author Hewlett-Packard Development Company, L.P.
 *
 * @section LICENSE
 *
 *      Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @section Contributors
 * @authors
 * Kyle Maas <kylemaasdev@gmail.com>
 *
 */




#ifndef WINDOWEDWEBAPP_H
#define WINDOWEDWEBAPP_H

#include "Common.h"

#include <webkitpalmtimer.h>
#include <list>

#include "AsyncCaller.h"
#include "GraphicsDefs.h"
#include "WebAppBase.h"
#include "sptr.h"
#include "Event.h"
#include "Timer.h"
#include "Window.h"
#include <PIpcChannelListener.h>
#include <PIpcBuffer.h>
#include <QRect>

class SysMgrKeyEvent;
class QKeyEvent;
class SysMgrTouchEvent;
class RemoteWindowData;
class WindowMetaData;
namespace Palm {
	class WebGLES2Context;
}

/**
 * Base UI functionality for app container classes to derive from.
 */
class WindowedWebApp : public WebAppBase , public PIpcChannelListener
{
public:
	/**
	 * Constructs a new instance of this class
	 * 
	 * WindowedWebApp provides base functionality
	 * for talking with the OS and other UI
	 * components for any kind of an app or other
	 * HTML/JS-driven system component.  If you
	 * need something to display content on the
	 * screen, this is probably the class you'll
	 * want to derive from.
	 * 
	 * @param	width			Width (in pixels) of the new window to display in.
	 * @param	height			Height (in pixels) of the new window to display in.
	 * @param	type			What type of window to create.
	 * @param	channel			IPC channel to send and receive messages on.
	 */
	WindowedWebApp(int width, int height, Window::Type type, PIpcChannel *channel = 0);
	
	/**
	 * Cleans up this instance
	 * 
	 * Cleans up the app.  Stops the redraw
	 * timer, and if it's a child card of another
	 * app, it sends the parent app a
	 * ViewHost_RemoveWindow IPC message and calls
	 * {@link WebAppManager::windowedAppRemoved() WebAppManager::windowedAppRemoved()}
	 * to let it know that we're closed.
	 */
	virtual ~WindowedWebApp();

	//Documented in parent
	virtual void attach(WebPage* page);
	
	/**
	 * Manually draw the contents of this app to the back buffer
	 * 
	 * - Stops the normal redraw timer.
	 * - If transparent, clears the image to a transparent color so the alpha channel's filled with zeros.
	 * - Draws the content (clipped to m_paintRect).
	 * - If page statistics are requested, draws them.
	 * - Starts the normal redraw timer again.
	 * 
	 * @param	clipToWindow		Whether or not to clip the contents to the window border (presumably).  Currently unused.
	 */
	virtual void paint(bool clipToWindow);
	
	/**
	 * Process a pen/mouse/finger input event
	 * 
	 * This method takes an input event from IPC
	 * and processes it.  For example, it takes
	 * pen events and translates them into
	 * simulated mouse events for JavaScript
	 * event processing in the app.
	 * 
	 * Recognized events are interpreted and
	 * passed to specific methods of the page's
	 * view object, such as pen, mouse, and
	 * gesture events.
	 * 
	 * In the case of a PenSingleTap event in an
	 * app that supports Mojo, a small chunk of
	 * JavaScript is run to feed the event to
	 * Mojo to be dispatched to the app.
	 * 
	 * Can be indirectly called by an IPC
	 * View_InputEvent message.
	 * 
	 * @todo Remove use of {@link getAveragedGesture()} since, without {@link recordGesture()} being used, it's rather pointless.
	 * 
	 * @param	e			Smart pointer to the event to process.
	 */
	virtual void inputEvent(sptr<Event> e);
	
	/**
	 * Process a keyboard input event
	 * 
	 * This method takes an input event from IPC
	 * and processes it.
	 * 
	 * Converts hardware keys like NavBack and
	 * the Orange key to corresponding
	 * recognized keyboard keys, then passes the
	 * key event to WebKit for processing by the
	 * app.
	 * 
	 * @param	e			Qt key event to process.
	 */
	virtual void keyEvent(QKeyEvent* e);
	
	/**
	 * Sets the focus state of this app
	 * 
	 * This method is called by IPC and a couple
	 * other places to let us know whether or not
	 * this app has the system focus.  It also
	 * sets the initial state of the editor if
	 * we're focused and a leaf app.
	 * 
	 * This method then lets the HTML/JS app
	 * contents and the Mojo framework know.
	 * 
	 * If we have acquired focus and Mojo is
	 * active in our app, this method calls
	 * Mojo.stageActivated() to let it know.
	 * 
	 * If we are losing focus and Mojo is
	 * active in our app, this method calls
	 * Mojo.stageDeactivated() to let it know.
	 * 
	 * @param	focused			True if this app has focus, false otherwise.
	 */
	virtual void focusedEvent(bool focused);
	
	/**
	 * Resizes this app
	 * 
	 * This method allows other modules to let
	 * us know that we need to resize.
	 * 
	 * Can be called indirectly via View_Resize
	 * or View_SyncResize IPC messages.
	 * 
	 * If a resize is needed (meaning the app
	 * is loaded and we're being resized to a
	 * different size than we were already at),
	 * this method performs the following steps:
	 * 
	 * - Resizes the HTML/CSS of the app.
	 * - Resizes the back buffer.
	 * - Redraws the app.
	 * - Lets WebAppManager know we have a new back buffer.
	 * 
	 * @param	newWidth		New width (in pixels) that our app should resize to.
	 * @param	newHeight		New height (in pixels) that our app should resize to.
	 * @param	resizeBuffer		Whether or not to resize the back buffer.
	 * @return				New back buffer key if it needed to be resized or -1 if the back buffer did not need to be resized
	 */
	virtual int resizeEvent(int newWidth, int newHeight, bool resizeBuffer);
	
	/**
	 * Flips the app's orientation from horizontal to vertical and vise versa
	 * 
	 * Flips the app contents' back buffer to the
	 * front buffer and displays it.  Repaints all
	 * other graphics associated with this app which
	 * are not part of the content of the app
	 * (card decorations, etc.).
	 * 
	 * Exactly the same as
	 * {@link WindowedWebApp::asyncFlipEvent() WindowedWebApp::asyncFlipEvent()}
	 * except that it doesn't call a callback when
	 * done.  In other words, this is the method
	 * you'd want to use if you don't need to know
	 * when it's done flipping orientations.
	 * 
	 * Can be called indirectly by IPC via a
	 * View_Flip IPC call.
	 * 
	 * @param	newWidth		Unused.
	 * @param	newHeight		Unused.
	 */
	virtual void flipEvent(int newWidth, int newHeight);
	
	/**
	 * Flips the app's orientation from horizontal to vertical and vise versa, then sends a ViewHost_AsyncFlipCompleted message.
	 * 
	 * Flips the app contents' back buffer to the
	 * front buffer and displays it.  Repaints all
	 * other graphics associated with this app which
	 * are not part of the content of the app
	 * (card decorations, etc.).
	 * 
	 * Exactly the same as
	 * {@link WindowedWebApp::flipEvent() WindowedWebApp::flipEvent()}
	 * except that when complete, it sends an async
	 * ViewHost_AsyncFlipCompleted IPC message.  In
	 * other words, this is the method you'd want to
	 * use if you need to know when it's done
	 * flipping orientations.
	 * 
	 * Can be called indirectly by IPC via a
	 * View_AsyncFlip IPC call.
	 * 
	 * @todo Change this to call WindowedWebApp::flipEvent(), since internally they're pretty much the same except for the callback.
	 * 
	 * @param	newWidth		Unused internally.  Passed through unchanged to the completion handler.
	 * @param	newHeight		Unused internally.  Passed through unchanged to the completion handler.
	 * @param	newScreenWidth		Unused internally.  Passed through unchanged to the completion handler.
	 * @param	newScreenHeight		Unused internally.  Passed through unchanged to the completion handler.
	 */
	virtual void asyncFlipEvent(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight);
	
	//Documented in parent
	virtual bool isWindowed() const { return true; }
	
	//Documented in parent
	virtual bool isCardApp() const { return false; }
	
	//Documented in parent
	virtual bool isDashboardApp() const { return false; }
	
	/**
	 * Checks if this app is a leaf app
	 * 
	 * Since many types of apps can be derived
	 * from WindowedWebApp, this tells you a bit
	 * about specifically what type this app is.
	 * 
	 * Since nothing currently returns anything
	 * other than true, this method is
	 * probably not going to be of much use
	 * right now.
	 * 
	 * @return				True if this app is a leaf app, false otherwise.
	 */
	virtual bool isLeafApp() const { return true; }
	
	/**
	 * Gets the type of window our app displays in
	 * 
	 * This can be used to distinguish between,
	 * for example, cards, banner alerts, status
	 * items, etc.  See {@link Window::Type} for
	 * more information.
	 * 
	 * @see Window::Type
	 * 
	 * @return				Type of window our app displays in.
	 */
	virtual Window::Type windowType() const { return m_winType; };
	
	/**
	 * Process an orientation change event
	 * 
	 * This method should be called any time the
	 * devices changes orientation (to rotate
	 * the screen, etc.).
	 * 
	 * If UI rotation is not locked (see
	 * {@link Settings::displayUiRotates}
	 * ), this method performs the following
	 * steps, as needed:
	 * 
	 * - Changes our internal orientation state.
	 * - Resizes the window, back buffer, and app contents to the new orientation.
	 * - If the app supports Mojo, fires off a Mojo orientation change event.
	 * 
	 * @param	orient			Orientation change event to process.
	 */
	virtual void setOrientation(Event::Orientation orient) {}
	
	/**
	 * Signals the rendering system that the entire app display needs to be redrawn
	 * 
	 * Adds the entire app display area to the
	 * rendering queue so that the next time the
	 * app is redrawn, the rendering system draws
	 * the whole app instead of just part of it.
	 * 
	 * To save on processing devoted to rendering,
	 * only call this method if the entire display
	 * (not just a smaller part of it) needs to be
	 * drawn.  If you're calling it from a derived
	 * class, please consider calling
	 * invalContents() instead so you're not
	 * spending the time to redraw everything.
	 */
	virtual void invalidate();
	
	/**
	 * Checks whether, as of our last update, our app has the system focus
	 * 
	 * We are periodically notified via IPC
	 * whether or not our app has the system
	 * focus (main app on the screen).  This
	 * method returns the state as of our latest
	 * update.
	 * 
	 * @return				True if this app has the system focus, false otherwise.
	 */
	virtual bool isFocused() const { return m_focused; }
	
	/**
	 * Finish up a transition
	 * 
	 * Should be called any time we had a scene
	 * transition that finishes.
	 * 
	 * Can be called indirectly via
	 * View_SceneTransitionFinished IPC message.
	 * 
	 * If needed, performs the following steps:
	 * 
	 * - Cleans up any resources allocated for the transition.
	 * - Redraws the app to make sure we've got a clean copy with no transition effects applied.
	 * - If Mojo is active in our app, runs Mojo.sceneTransitionCompleted() to let our app know the transition is complete.
	 */
	virtual void sceneTransitionFinished() {}
	
	/**
	 * Indirectly calls OverlayWindowManager::applyLaunchFeedback()
	 * 
	 * @todo Document this once OverlayWindowManager is documented.
	 * 
	 * @see OverlayWindowManager::applyLaunchFeedback()
	 */
	virtual void applyLaunchFeedback(int cx, int cy);
	
	/**
	 * Gets our current app width
	 * 
	 * Our app has to track the size of itself to
	 * be able to draw correctly.  This gets the
	 * current the size we're using for our app
	 * (in pixels).  This is also the same size
	 * that we're current using for a back buffer.
	 * 
	 * Make sure to check this just before using
	 * it, as this size can and does change when
	 * our app is resized.
	 * 
	 * @return				Width of our app, in pixels.
	 */
	int windowWidth() const { return m_windowWidth; }
	
	/**
	 * Gets our current app height
	 * 
	 * Our app has to track the size of itself to
	 * be able to draw correctly.  This gets the
	 * current the size we're using for our app
	 * (in pixels).  This is also the same size
	 * that we're current using for a back buffer.
	 * 
	 * Make sure to check this just before using
	 * it, as this size can and does change when
	 * our app is resized.
	 * 
	 * @return				Height of our app, in pixels.
	 */
	int windowHeight() const { return m_windowHeight; }
	
	/**
	 * Dispatches incoming IPC messages and calls the class methods associated with them
	 * 
	 * The following is a list of the mssages we
	 * handle and which methods they call:
	 * 
	 * - View_Focus -> {@link focusedEvent()}
	 * - View_Resize -> {@link onResize()}
	 * - View_SyncResize -> {@link onSyncResize()}
	 * - View_InputEvent -> {@link onInputEvent()}
	 * - View_KeyEvent -> {@link onKeyEvent()}
	 * - View_TouchEvent -> {@link onTouchEvent()}
	 * - View_Close -> {@link onClose()}
	 * - View_DirectRenderingChanged -> {@link onDirectRenderingChanged()}
	 * - View_SceneTransitionFinished -> {@link onSceneTransitionFinished()}
	 * - View_ClipboardEvent_Cut -> {@link onClipboardEvent_Cut()}
	 * - View_ClipboardEvent_Copy -> {@link onClipboardEvent_Copy()}
	 * - View_ClipboardEvent_Paste -> {@link onClipboardEvent_Paste()}
	 * - View_SelectAll -> {@link onSelectAll()}
	 * - View_Flip -> {@link onFlip()}
	 * - View_AsyncFlip -> {@link onAsyncFlip()}
	 * - View_AdjustForPositiveSpace -> {@link onAdjustForPositiveSpace()}
	 * - View_KeyboardShown -> {@link onKeyboardShow()}
	 * - View_SetComposingText -> {@link onSetComposingText()}
	 * - View_CommitComposingText -> {@link onCommitComposingText()}
	 * - View_CommitText -> {@link onCommitText()}
	 * - View_PerformEditorAction -> {@link onPerformEditorAction()}
	 * - View_RemoveInputFocus -> {@link onRemoveInputFocus()}
	 * 
	 * @param	msg		IPC message to dispatch.
	 */
	virtual void onMessageReceived(const PIpcMessage& msg);
	
	/**
	 * Unknown at this time
	 * 
	 * @todo Determine if there is any purpose for this, and if there is found to be no definitive purpose, remove it.
	 */
	virtual void onDisconnected();
	
	//Documented in parent
	virtual int  getKey() const;
	
	/**
	 * Gets the back buffer key
	 * 
	 * @see RemoteWindowData::key()
	 * 
	 * @todo Document this further once RemoteWindowData::key() is documented.
	 * 
	 * @return			Back buffer key.
	 */
	int routingId() const;
	
	/**
	 * IPC buffer ID for the WindowMetaData object for our window
	 * 
	 * Returns the ID of the shared memory buffer
	 * containing this window's WindowMetaData
	 * structure.  This can then be used by
	 * another thread/process to access said
	 * buffer and read from/write to it.
	 * 
	 * @todo Document this further once WindowMetaData is documented.
	 * 
	 * @return			Buffer ID of out WindowMetaData structure.
	 */
	int metadataId() const;
	
	/**
	 * IPC message handler for View_Resize messages
	 * 
	 * Used to notify us that we should resize
	 * but that the caller doesn't need the back
	 * buffer key.
	 * 
	 * Allows calling of {@link resizeEvent()} by
	 * other processes via IPC.
	 * 
	 * See {@link resizeEvent()} for more details.
	 * 
	 * @see resizeEvent()
	 * 
	 * @param	newWidth		New width (in pixels) that our app should resize to.
	 * @param	newHeight		New height (in pixels) that our app should resize to.
	 * @param	resizeBuffer		Whether or not to resize the back buffer.
	 */
	virtual void onResize(int width, int height, bool resizeBuffer);
	
	/**
	 * IPC message handler for View_Flip
	 * 
	 * Allows other processes to ask us to flip
	 * the back buffer and redraw, essentially
	 * updating the display for our app.
	 * 
	 * @see {@link flipEvent()} for more details.
	 * 
	 * @param	newWidth		Unused.
	 * @param	newHeight		Unused.
	 */
	virtual void onFlip(int newWidth, int newHeight);
	
	/**
	 * IPC message handler for View_AsyncFlip messages
	 * 
	 * Dispatches the info to asyncFlipEvent().
	 * Please see that method for details.
	 * 
	 * @see asyncFlipEvent()
	 * 
	 * @param	newWidth	Unused internally. Passed through unchanged to the completion handler.
	 * @param	newHeight	Unused internally. Passed through unchanged to the completion handler.
	 * @param	newScreenWidth	Unused internally. Passed through unchanged to the completion handler.
	 * @param	newScreenHeight	Unused internally. Passed through unchanged to the completion handler.
	 */
	virtual void onAsyncFlip(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight);
	
	/**
	 * IPC message handler for View_SyncResize messages
	 * 
	 * Used to notify us that we should resize
	 * and that the caller wants the key for the
	 * back buffer.
	 * 
	 * Allows calling of {@link resizeEvent()} by
	 * other processes via IPC.
	 * 
	 * See {@link resizeEvent()} for more details.
	 * 
	 * @see resizeEvent()
	 * 
	 * @param	newWidth		New width (in pixels) that our app should resize to.
	 * @param	newHeight		New height (in pixels) that our app should resize to.
	 * @param	resizeBuffer		Whether or not to resize the back buffer.
	 * @param	newKey			Pointer to int to place the new back buffer key into.
	 */
	virtual void onSyncResize(int width, int height, bool resizeBuffer, int* newKey);
	
	/**
	 * IPC message handler for View_AdjustForPositiveSpace messages
	 * 
	 * If Mojo is active in our app, dispatches
	 * the event to Mojo as
	 * positiveSpaceChanged().
	 * 
	 * @todo Document width and height.
	 * 
	 * @param	width		Specifics are unknown at this time as this is directly passed through to Mojo.
	 * @param	height		Specifics are unknown at this time as this is directly passed through to Mojo.
	 */
	virtual void onAdjustForPositiveSpace(int width, int height);
	
	/**
	 * IPC message handler for View_KeyboardShown messages
	 * 
	 * Given to us whenever the on-screen keyboard
	 * is either shown or hidden, with the value of
	 * val indicating whether it is now shown or
	 * hidden.
	 * 
	 * If Mojo is active in our app, dispatches
	 * the event to Mojo as
	 * keyboardShown().
	 * 
	 * @param	val		Whether or not the keyboard is now visible.
	 */
	virtual void onKeyboardShow(bool val);
	
	/**
	 * IPC message handler for View_Close messages
	 * 
	 * Called whenever our app is being asked to close.
	 * 
	 * @param	disableKeepAlive	True to force the app to quit, false to allow it to quit gracefully.
	 */
	virtual void onClose(bool disableKeepAlive);
	
	/**
	 * Take a View_InputEvent message from IPC and feed it to inputEvent()
	 * 
	 * Pulls the event out of the IPC message and
	 * feeds it to inputEvent().  For more details,
	 * please see the documentation for inputEvent().
	 * 
	 * @see inputEvent()
	 * 
	 * @param	wrapper		Event wrapper parameter from the IPC message.
	 */
	virtual void onInputEvent(const SysMgrEventWrapper& wrapper);
	
	/**
	 * IPC message handler for View_KeyEvent messages
	 * 
	 * Translates an incoming event to a Qt event
	 * and calls {@link keyEvent()}.
	 * 
	 * Allows calling of {@link keyEvent()} by
	 * other processes via IPC.
	 * 
	 * See {@link keyEvent()} for more details.
	 * 
	 * @see keyEvent()
	 * 
	 * @param	keyEvent	Keyboard event to process.
	 */
	virtual void onKeyEvent(const SysMgrKeyEvent& keyEvent);
	
	/**
	 * IPC message handler for View_TouchEvent messages
	 * 
	 * If the app has requested touch events
	 * ({@link WebPage::isTouchEventsNeeded()}),
	 * translate the Qt event to Palm event
	 * structures/classes and pass them to the
	 * app.
	 * 
	 * @param	touchEvent	Touch event to process.
	 */
	virtual void onTouchEvent(const SysMgrTouchEvent& touchEvent);
	
	/**
	 * IPC message handler for View_DirectRenderingChanged messages
	 * 
	 * Called whenever the system needs us to
	 * change from direct rendering to not or
	 * vise versa.
	 * 
	 * Since we don't get a parameter telling
	 * us what to change to, it's up to us to
	 * check the setting for direct rendering
	 * and change our rendering mode to match.
	 */
	virtual void onDirectRenderingChanged();
	
	/**
	 * IPC message handler for View_SceneTransitionFinished messages
	 * 
	 * Called when a running transition is
	 * finished, to allow us to clean up any
	 * resources used in the process.
	 * 
	 * Allows calling of
	 * {@link sceneTransitionFinished()} by
	 * other processes via IPC.
	 * 
	 * See {@link sceneTransitionFinished()}
	 * for more details.
	 * 
	 * @see sceneTransitionFinished()
	 */
	virtual void onSceneTransitionFinished();
	
	/**
	 * IPC messasge handler for View_ClipboardEvent_Cut messages
	 * 
	 * Passes the Cut action to the app,
	 * which should handle it.
	 * 
	 * @see WebPage::cut()
	 */
	virtual void onClipboardEvent_Cut();
	
	/**
	 * IPC messasge handler for View_ClipboardEvent_Copy messages
	 * 
	 * Passes the Copy action to the app,
	 * which should handle it.
	 * 
	 * @see WebPage::copy()
	 */
	virtual void onClipboardEvent_Copy();
	
	/**
	 * IPC messasge handler for View_ClipboardEvent_Paste messages
	 * 
	 * Passes the Paste action to the app,
	 * which should handle it.
	 * 
	 * @see WebPage::paste()
	 */
	virtual void onClipboardEvent_Paste();
	
	/**
	 * IPC messasge handler for View_SelectAll messages
	 * 
	 * Passes the Select All action to the
	 * app, which should handle it.
	 * 
	 * @see WebPage::selectAll()
	 */
	virtual void onSelectAll();
	
	/**
	 * Unknown at this time
	 * 
	 * @todo Document this further once WebPage::setComposingText() is documented.
	 */
	virtual void onSetComposingText(const std::string& text);
	
	/**
	 * Unknown at this time
	 * 
	 * @todo Document this further once WebPage::commitComposingText() is documented.
	 */
	virtual void onCommitComposingText();
	
	/**
	 * Unknown at this time
	 * 
	 * @todo Document this further once WebPage::commitText() is documented.
	 */
	virtual void onCommitText(const std::string& text);
	
	/**
	 * Unknown at this time
	 * 
	 * @todo Document this further once WebPage::performEditorAction() is documented.
	 */
	virtual void onPerformEditorAction(int action);
	
	/**
	 * IPC message handler for View_RemoveInputFocus messages
	 * 
	 * Called to ask us to remove focus from
	 * input areas within our app.
	 * 
	 * Allows calling of
	 * {@link Page::removeInputFocus()} by
	 * other processes via IPC.
	 * 
	 * See {@link Page::removeInputFocus()}
	 * for more details.
	 * 
	 * @see Page::removeInputFocus()
	 */
	virtual void onRemoveInputFocus();
	
	//Documented in parent
	virtual void windowSize(int& width, int& height);
	
	//Documented in parent
	virtual void screenSize(int& width, int& height);
	
	/**
	 * Changes our window properties
	 * 
	 * Changed our window properties and
	 * sends a message via IPC to let
	 * the view host know that our
	 * properties have changed and to
	 * what.
	 * 
	 * @todo Document this further once WindowProperties is documented.
	 * 
	 * @param	winProp		New window properties to use.
	 */
	virtual void setWindowProperties(WindowProperties &winProp);
	
	/**
	 * Resume scheduled redrawing
	 * 
	 * @see WindowedWebApp::displayOn()
	 */
	virtual void displayOn() {}
	
	/**
	 * Suspend all scheduled redrawing until further notice
	 * 
	 * @see WindowedWebApp::displayOff()
	 */
	virtual void displayOff() {}
	
	//Documented in parent
	virtual Palm::WebGLES2Context* getGLES2Context();

protected:

	/**
	 * Initializes the window for the app
	 * 
	 * Call this from subclasses after setting width and height.
	 * 
	 * This method sets up the rendering system, connects
	 * it to IPC, and enables/disables direct rendering.
	 * 
	 * Sequence is as follows:
	 * 
	 * - Sets up WindowedWebApp::m_data as a RemoteWindowDataFactory.
	 * - Sets m_data's IPC channel to the one we're running on.
	 * - Creates an IPC buffer m_metaDataBuffer and m_metaData to give to m_data.
	 * - Calls m_data's RemoteWindowData::setWindowMetaDataBuffer() to set its data buffer.
	 * - If this is a card, enables direct rendering for m_data, otherwise it disables it.
	 * 
	 * @note If this app is a card, this method enables direct rendering.  Otherwise, it is disabled.
	 */
	void init();

	//Documented in parent
	virtual void focus();
	
	//Documented in parent
	virtual void unfocus();
	
	//Documented in parent
	virtual void invalContents(int x, int y, int width, int height);
	
	/**
	 * Currently unused
	 * 
	 * Re-paints the entire app next redraw.
	 * 
	 * @todo Determine if this needs to exist anymore, as it is no longer called from anywhere.
	 * 
	 * @param	newContentsX		Unknown - currently unused.
	 * @param	newContentsY		Unknown - currently unused.
	 */
	virtual void scrollContents(int newContentsX, int newContentsY);
	
	//Documented in parent
	virtual void loadFinished();
	
	//Documented in parent
	virtual void stagePreparing();
	
	//Documented in parent
	virtual void stageReady();
	
	//Documented in parent
	virtual void editorFocusChanged(bool focused, const PalmIME::EditorState& state);
	
	//Documented in parent
	virtual void autoCapEnabled(bool enabled);
	
	//Documented in parent
	virtual void needTouchEvents(bool needTouchEvents);
	
	/**
	 * Builds a JSON string based on properties contained in winProp
	 * 
	 * JSON object may contain any of the following
	 * properties depending on winProp.flags:
	 * 
	 * - blockScreenTimeout
	 * - subtleLightbar
	 * - fullScreen
	 * - activeTouchpanel
	 * - alsDisabled
	 * - enableCompass
	 * - enableGyro
	 * - overlayNotificationsPosition
	 * - suppressBannerMessages
	 * - hasPauseUi
	 * - suppressGestures
	 * - webosDragMode
	 * - dashHeight
	 * - statusBarColor
	 * - rotationLockMaximized
	 * - allowResizeOnPositiveSpaceChange
	 * 
	 * @param	winProp			Window properties to read from.
	 * @param	propString		String to return JSON object into.
	 */
	virtual void getWindowPropertiesString(WindowProperties &winProp, std::string &propString) const;
	
protected:
	
	/**
	 * Queues a repaint of our app
	 * 
	 * Starts a WebKit timer to redraw our
	 * app as soon as it's able to process
	 * it (timer with a timeout of 0ms).
	 * This allows it to neatly fall into
	 * the event queue such that it
	 * processes in order.
	 * 
	 * Has no effect if our app is currently
	 * being closed or if we're already
	 * waiting for a previously-queued
	 * repaint timer to fire.
	 */
	void startPaintTimer();
	
	/**
	 * Stops queued repaint timer
	 * 
	 * Stops the timer previously set by
	 * {@link startPaintTimer()}, if
	 * there is one.  If not, this method
	 * has no effect.
	 */
	void stopPaintTimer();
	
	/**
	 * Possible states of app focus
	 */
	enum PendingFocus {
		/**
		 * Indicates that we have not yet received a focus event while the app is loading
		 */
		PendingFocusNone = 0,
		
		/**
		 * Indicates that while the app was still loading we were informed that our app currently has the system focus
		 */
		PendingFocusTrue,
		
		/**
		 * Indicates that while the app was still loading we were informed that our app currently does not have the system focus
		 */
		PendingFocusFalse
	};
	
	/**
	 * Back buffer that we draw to
	 */
	RemoteWindowData* m_data;
	
	/**
	 * IPC buffer for our WindowMetaData structure so it can be passed between threads/processes safely
	 * 
	 * @todo Document this further once WindowMetaData is documented.
	 */
	PIpcBuffer* m_metaDataBuffer;
	
	/**
	 * Pointer to our WindowMetaData structure
	 * 
	 * @todo Document this further once WindowMetaData is documented.
	 */
	WindowMetaData* m_metaData;
	
	/**
	 * Repaint timer used by {@link startPaintTimer()}
	 * 
	 * See {@link startPaintTimer()} for usage.
	 * 
	 * @see startPaintTimer()
	 */
	WebKitPalmTimer*	m_paintTimer;
	
	/**
	 * Type of window our app resides in
	 * 
	 * @see Window::Type
	 */
	Window::Type		m_winType;
	
	/**
	 * Current width, in pixels, of our back buffer
	 * 
	 * Should always track with and be exactly
	 * equal to {@link m_windowWidth}.
	 */
	int					m_width;
	
	/**
	 * Current width, in pixels, of our back buffer
	 * 
	 * Should always track with and be exactly
	 * equal to {@link m_windowHeight}.
	 */
	int					m_height;
	
	/**
	 * Whether or not our app is currently cleaning up in the destructor.
	 * 
	 * This is mainly used so we don't try to
	 * redraw the app while we're being told to
	 * stop.  There's no point in drawing if
	 * we're actively being asked to stop
	 * running.
	 */
	bool                m_beingDeleted;
	
	/**
	 * Whether or not our app is currently preparing the stage (still loading the app)
	 */
	bool                m_stagePreparing;
	
	/**
	 * Whether or not our app is completely loaded
	 */
	bool                m_stageReady;
	
	/**
	 * Whether or not our app has been added to the window manager via an IPC call from {@link loadFinished()}
	 */
	bool				m_addedToWindowMgr;
	
	/**
	 * Current width, in pixels, of our app
	 * 
	 * Available publicly via windowWidth().
	 * 
	 * Should always track with and be exactly
	 * equal to {@link m_width}.
	 */
	uint32_t            m_windowWidth;
	
	/**
	 * Current width, in pixels, of our app
	 * 
	 * Available publicly via windowHeight().
	 * 
	 * Should always track with and be exactly
	 * equal to {@link m_height}.
	 */
	uint32_t            m_windowHeight;
	
	/**
	 * Area of our app which needs to be redrawn next time {@link paint()} runs
	 */
	QRect m_paintRect;

	/**
	 * Keeps track on how many PenDown's we blocked
	 */
	int  m_blockCount;
	
	/**
	 * Whether or not to block PenDown events
	 * 
	 * @see m_blockCount
	 * @see WindowedWebApp::inputEvent
	 */
	bool m_blockPenEvents;
	
	/**
	 * Time that the latest gesture event ended
	 * 
	 * Set by WindowedWebApp::inputEvent()
	 * 
	 * @see WindowedWebApp::inputEvent()
	 */
	uint32_t m_lastGestureEndTime;
	
	/**
	 * Whether or not to show memory statistics overlayed onto the app's display
	 * 
	 * Toggled via a Alt->Percent KeyPress event in WindowedWebApp::keyEvent().
	 */
	bool m_showPageStats;
	
	/**
	 * Whether or not our app is focused or unfocused
	 * 
	 * Similar to m_pendingFocus, but tracked even
	 * if the app has already loaded.
	 */
	bool m_focused;
	
	/**
	 * Whether or not our app is focused or unfocused
	 * 
	 * Used as temporary storage while we're still
	 * loading the page contents so that once they
	 * load, we can notify the app itself of the
	 * focus state.
	 * 
	 * Similar to m_focused, but only tracked when
	 * the app is still loading.  After the app has
	 * loaded, the state of this variable remains
	 * the same as when it finished loading.
	 */
	PendingFocus m_pendingFocus;
	
	/**
	 * Unknown - currently unused
	 * 
	 * @todo Determine if this can be removed entirely.
	 */
	NativeGraphicsSurface* m_metaCaretHint;
	
	/**
	 * Timer which is set to run while we're loading the contents of the app
	 * 
	 * If our app doesn't load quickly enough,
	 * this timer ensures that we still register
	 * our app with the window manager.
	 * 
	 * When triggered, calls {@link showWindowTimeout()}.
	 * 
	 * @see showWindowTimeout()
	 */
	Timer<WindowedWebApp> m_showWindowTimer;
	
	/**
	 * Whether currently-executing touch events should be translated to mouse events
	 * 
	 * Used by {@link onTouchEvent()} to
	 * determine whether a series of touch
	 * events should be translated to mouse
	 * events.
	 */
	bool m_generateMouseClick;
	
	/**
	 * Window properties for the current window
	 * 
	 * @todo Document this further once {@link WindowProperties} and its uses are documented.
	 */
	WindowProperties m_winProps;

protected:
	/**
	 * Renders various debugging statistics layered over the top of our app
	 * 
	 * Sequence of actions is as follows:
	 * - Asks WebKit for some page statistics.
	 * - Loads font from /usr/share/fonts/Prelude-Bold.ttf
	 * - Gets the rendering context to draw to from {@link m_data}.
	 * - If context getting was successful, renders a statistics report.
	 * - Logs statistics report to the debug log.
	 * 
	 * Statistics include:
	 * - Number of images used in the app and how much memory they're consuming.
	 * - Number of DOM nodes.
	 * - Number of render nodes.
	 * - Number of render layers.
	 * 
	 * @param	offsetX			X coordinate to draw text at.
	 * @param	offsetY			Y coordinate to draw text at.
	 */
	void renderPageStatistics(int offsetX=0, int offsetY=0);
	
	/**
	 * Unknown at this time
	 * 
	 * @todo This method is currently undefined and unused.  Research whether this can be removed, and it so, remove it.
	 */
	void renderMetaHint(int offsetX=0, int offsetY=0);
	
	/**
	 * Adds our app to the window manager
	 * 
	 * Used by {@link m_showWindowTimer} to
	 * ensure that even if our app takes a
	 * while to load, we still paint something
	 * (even if the app's not done loading yet)
	 * and add ourselves to the window manager.
	 * 
	 * @return				Always returns false.
	 */
	bool showWindowTimeout();
	
	/**
	 * Returns whether or not the initial page content of this app is completely loaded and ready to run
	 * 
	 * If the stage is still preparing or is somehow
	 * just not ready, this method will return false.
	 * 
	 * @return				True if the app is completely loaded, false otherwise.
	 */
	bool appLoaded() const;
	
	/**
	 * Checks whether the background of this app should be transparent when drawn
	 * 
	 * @return				True if transparent, false otherwise.
	 */
	bool isTransparent() const;
	
private:

	/**
	 * Holds information about the orientation and size of a gesture
	 * 
	 * @todo This structure is rendered rather pointless by the unuse of {@link recordGesture()}.  Please research whether it can be removed entirely along with associated functionality.
	 */
	struct RecordedGestureEntry {
		/**
		 * Constructs a RecordedGestureEntry object
		 * 
		 * @param	s		Scale of gesture.
		 * @param	r		Rotation of gesture.
		 * @param	cX		Center X of gesture.
		 * @param	cY		Center Y of gesture.
		 */
		RecordedGestureEntry(float s, float r, int cX, int cY)
			: scale(s)
			, rotate(r)
			, centerX(cX)
			, centerY(cY) {
		}
		
		/**
		 * Scale of gesture
		 */
		float scale;
		
		/**
		 * Rotation of gesture
		 */
		float rotate;
		
		/**
		 * Center X of gesture
		 */
		int centerX;
		
		/**
		 * Center Y of gesture
		 */
		int centerY;
	};
	
	/**
	 * List of orientation and size of the most recent gestures recorded by {@link recordGesture()}
	 * 
	 * @todo This list is rendered rather useless by {@link recordGesture()} never being called anywhere.  Determine if {@link recordGesture()} can be removed and, if so, remove this list and associated functionality.
	 */
	std::list<RecordedGestureEntry> m_recordedGestures;	
	
	/**
	 * Adds information about a gesture to our list so we can average them
	 * 
	 * We currently hold a list of some rather
	 * basic information about gestures so we
	 * can return an "average" gesture with
	 * {@link getAveragedGesture()}.
	 * 
	 * However, this method is currently
	 * unused, making it rather pointless.
	 * 
	 * @note Unused at this time.
	 * 
	 * @todo This method is currently unused.  Determine if it can be removed, along with other associated functionality rendered mostly useless by this being gone such as {@link getAveragedGesture()}.
	 * 
	 * @param	s			Scale of gesture.
	 * @param	r			Rotation of gesture.
	 * @param	cX			Center X coordinate of gesture.
	 * @param	cY			Center Y coordinate of gesture.
	 */
	void recordGesture(float s, float r, int cX, int cY);
	
	/**
	 * Gets the average orientation and size for gestures recorded by {@link recordGesture()}
	 * 
	 * Goes through our list of recorded
	 * gestures and averages components of
	 * them to get an "average" gesture size.
	 * 
	 * @note Currently only used by {@link WindowedWebApp::inputEvent()}.
	 * 
	 * @todo This method is rendered rather useless by {@link recordGesture()} being left unused.  Determine if {@link recordGesture()} can be removed and, if so, remove this method and associated functionality.
	 * 
	 * @return				Average orientation and position of recorded gestures.
	 */
	RecordedGestureEntry getAveragedGesture() const;
	
	/**
	 * Translates key events from hardware buttons to gestures
	 * 
	 * This method translates a hardware
	 * button event such as a "Launcher"
	 * button to a gesture for that action.
	 * 
	 * Translation table includes:
	 * - Qt::Key_CoreNavi_Menu -> "forward"
	 * - Qt::Key_CoreNavi_Launcher -> "up"
	 * - Qt::Key_CoreNavi_SwipeDown -> "down"
	 * 
	 * This translated gesture is then
	 * fed to Mojo in our app (if our app
	 * uses Mojo) via Mojo.handleGesture().
	 * 
	 * @param	e			Key event to process into a gesture.
	 */
	void keyGesture(QKeyEvent* e);

private:	
	
	/**
	 * Operator to copy this instance to another instance
	 * 
	 * Defined here (as private) purely to
	 * block anyone from copying a
	 * WindowedWebApp instance without building
	 * one from scratch.
	 * 
	 * @param	app			Instance to copy from.
	 * @return				Instance copied to (our current instance).
	 */
	WindowedWebApp& operator=(const WindowedWebApp&);
	
	/**
	 * Copy constructor
	 * 
	 * Called when someone runs something
	 * like the following:
	 * 
	 * <code>
	 * WindowedWebApp a = new WindowedWebApp();
	 * WindowedWebApp b = a;
	 * </code>
	 * 
	 * Except that we block that by
	 * defining this method as private.
	 * This prevents someone from copying
	 * a WindowedWebApp without building
	 * one from scratch (there's too much
	 * initialization involved to support
	 * this).
	 * 
	 * @param	app			Instance to copy from.
	 */
	WindowedWebApp(const WindowedWebApp&);

	friend class WebAppBase;
	friend class JsSysObject;
};

#endif /* WINDOWEDWEBAPP_H */
