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

	virtual Window::Type windowType() const { return m_winType; };

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

	virtual void sceneTransitionFinished() {}
	
	/**
	 * Indirectly calls OverlayWindowManager::applyLaunchFeedback()
	 * 
	 * @todo Document this once OverlayWindowManager is documented.
	 * 
	 * @see OverlayWindowManager::applyLaunchFeedback()
	 */
	virtual void applyLaunchFeedback(int cx, int cy);

	int windowWidth() const { return m_windowWidth; }
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
	virtual void onDisconnected();
	
	//Documented in parent
	virtual int  getKey() const;
	
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

	virtual void onResize(int width, int height, bool resizeBuffer);
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
	virtual void onKeyboardShow(bool val);
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
	virtual void onKeyEvent(const SysMgrKeyEvent& keyEvent);
	virtual void onTouchEvent(const SysMgrTouchEvent& touchEvent);
	virtual void onDirectRenderingChanged();
	virtual void onSceneTransitionFinished();
	virtual void onClipboardEvent_Cut();
	virtual void onClipboardEvent_Copy();
	virtual void onClipboardEvent_Paste();
    virtual void onSelectAll();

	virtual void onSetComposingText(const std::string& text);
	virtual void onCommitComposingText();

	virtual void onCommitText(const std::string& text);

	virtual void onPerformEditorAction(int action);

	virtual void onRemoveInputFocus();

	virtual void windowSize(int& width, int& height);
	
	//Documented in parent
	virtual void screenSize(int& width, int& height);
	
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
    virtual void scrollContents(int newContentsX, int newContentsY);
	
	//Documented in parent
	virtual void loadFinished();
	virtual void stagePreparing();
	virtual void stageReady();
	
	//Documented in parent
	virtual void editorFocusChanged(bool focused, const PalmIME::EditorState& state);
	
	//Documented in parent
	virtual void autoCapEnabled(bool enabled);
	
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

	void startPaintTimer();
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

	WebKitPalmTimer*	m_paintTimer;
	Window::Type		m_winType;
	int					m_width;
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
	bool                m_stagePreparing;
	bool                m_stageReady;
	bool				m_addedToWindowMgr;

	uint32_t            m_windowWidth;
	uint32_t            m_windowHeight;

	QRect m_paintRect;

	int  m_blockCount; //Keeps track on how many PenDown's we blocked.
	bool m_blockPenEvents;
	uint32_t m_lastGestureEndTime;
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

	NativeGraphicsSurface* m_metaCaretHint;

	Timer<WindowedWebApp> m_showWindowTimer;

    bool m_generateMouseClick;

	WindowProperties m_winProps;

protected:
	
	void renderPageStatistics(int offsetX=0, int offsetY=0);
	void renderMetaHint(int offsetX=0, int offsetY=0);
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

	struct RecordedGestureEntry {
		RecordedGestureEntry(float s, float r, int cX, int cY)
			: scale(s)
			, rotate(r)
			, centerX(cX)
			, centerY(cY) {
		}
		
		float scale;
		float rotate;
		int centerX;
		int centerY;
	};

	std::list<RecordedGestureEntry> m_recordedGestures;	

	void recordGesture(float s, float r, int cX, int cY);
	RecordedGestureEntry getAveragedGesture() const;

	void keyGesture(QKeyEvent* e);

private:	
	
	WindowedWebApp& operator=(const WindowedWebApp&);
	WindowedWebApp(const WindowedWebApp&);

	friend class WebAppBase;
	friend class JsSysObject;
};

#endif /* WINDOWEDWEBAPP_H */
