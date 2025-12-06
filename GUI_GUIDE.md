# GUI Guide for All Platforms

## Overview

The Jobs distributed computing platform now features modern, user-friendly GUIs for all three supported platforms:
- **Android**: Material Design 3 with Jetpack Compose
- **Linux**: Qt-based desktop application
- **Windows**: Qt-based desktop application (same as Linux)

## Android GUI

### Features
- **Material Design 3**: Modern, clean interface
- **Real-time Updates**: Live resource monitoring and device discovery
- **Card-based Layout**: Organized information in cards
- **Color-coded Status**: Visual indicators for connection status
- **Touch-optimized**: Large buttons and easy navigation

### Screens
1. **Connection Tab**: Server address input and connection status
2. **Resource Monitor**: CPU, memory, GPU usage with progress bars
3. **Task Statistics**: Queued, completed tasks with decision history
4. **Network Discovery**: Device list with recruit/work buttons
5. **Server Mode Toggle**: Enable/disable server functionality

### UI Components
- Material 3 Cards with elevation
- Progress bars for resource usage
- Color-coded status indicators
- Device cards with action buttons
- Modern typography and spacing

## Linux/Windows GUI (Qt)

### Features
- **Qt Framework**: Cross-platform native look and feel
- **Dark Theme**: Modern dark color scheme (configurable)
- **Tabbed Interface**: Organized into logical sections
- **Real-time Monitoring**: Live resource updates
- **Network Discovery**: Visual device scanning and selection

### Tabs

#### 1. Network Discovery
- **Scan Network Button**: Start device discovery
- **Device List**: Shows discovered devices with details
- **Recruit/Work For Buttons**: Connect to devices
- **Status Indicators**: Shows scanning progress
- **Refresh**: Rescan network

#### 2. Connection
- **Server Address Input**: IP address and port
- **Connect/Disconnect Buttons**: Manage connections
- **Server Mode Checkbox**: Enable server functionality
- **Status Label**: Connection status with color coding

#### 3. Resources
- **CPU Usage**: Progress bar with percentage
- **Memory Usage**: Available memory display
- **CPU Cores**: Number of available cores
- **Auto-update**: Refreshes every second

#### 4. Tasks
- **Statistics Panel**: Queued, completed, failed counts
- **Task Table**: Detailed task list with status
- **Clear Button**: Remove completed tasks
- **Real-time Updates**: Task status changes

### Menu Bar
- **File Menu**: Quit application
- **View Menu**: Toggle server mode
- **Help Menu**: About dialog

### Status Bar
- Shows current operation status
- Updates in real-time

## Building GUIs

### Android
```bash
cd android
./gradlew build
./gradlew installDebug
```

### Linux/Windows (Qt)
```bash
cd worker_client_gui
mkdir build && cd build
cmake ..
make  # or nmake on Windows with MSVC
```

**Requirements:**
- Qt 5.15+ or Qt 6.x
- CMake 3.20+
- C++17 compiler

## UI Consistency

All three platforms share:
- **Same Functionality**: All features available on all platforms
- **Consistent Workflow**: Similar user experience
- **Real-time Updates**: Live data refresh
- **Visual Feedback**: Status indicators and progress bars
- **Error Handling**: User-friendly error messages

## Platform-Specific Features

### Android
- Material Design 3 components
- Touch gestures
- Mobile-optimized layout
- System integration (notifications, etc.)

### Linux/Windows
- Native window management
- Keyboard shortcuts
- System tray integration (future)
- File dialogs
- Native look and feel

## Customization

### Android
- Themes in `ui/theme/Theme.kt`
- Colors in `ui/theme/Color.kt`
- Typography in `ui/theme/Type.kt`

### Qt (Linux/Windows)
- Dark theme palette in `main.cpp`
- Stylesheets for custom styling
- QStyle for native appearance

## Future Enhancements

- System tray icons
- Notifications
- Settings/preferences dialogs
- Theme selection (light/dark)
- Customizable layouts
- Keyboard shortcuts
- Drag and drop
- Multi-window support

