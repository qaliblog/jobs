# CI/CD Workflows

This project includes GitHub Actions workflows for automated building and deployment of all platform versions.

## Workflows

### Android Build (`android-build.yml`)

Builds Android APK release version.

**Triggers:**
- Push to `main`/`master` branch (when Android files change)
- Pull requests (when Android files change)
- Manual workflow dispatch

**Features:**
- Sets up JDK 17
- Configures Android SDK
- Builds release APK
- Optional APK signing (if keystore secrets are configured)
- Uploads APK as artifact
- Creates GitHub release on tag push

**Secrets (optional for signing):**
- `ANDROID_KEYSTORE_BASE64`: Base64 encoded keystore file
- `ANDROID_KEYSTORE_PASSWORD`: Keystore password
- `ANDROID_KEY_ALIAS`: Key alias
- `ANDROID_KEY_PASSWORD`: Key password

**Artifacts:**
- `android-apk`: Release APK file

### Linux Build (`linux-build.yml`)

Builds Linux binaries for backend, worker client, and worker GUI.

**Triggers:**
- Push to `main`/`master` branch (when C++ files change)
- Pull requests (when C++ files change)
- Manual workflow dispatch

**Jobs:**
1. **build-backend**: Builds C++ backend server
2. **build-worker**: Builds CLI worker client
3. **build-worker-gui**: Builds Qt-based GUI worker client

**Features:**
- Installs build dependencies (CMake, OpenSSL)
- Installs Qt 6.5.0 for GUI builds
- Builds with parallel compilation
- Uploads binaries as artifacts

**Artifacts:**
- `linux-backend`: Backend server binary
- `linux-worker`: Worker client binary
- `linux-worker-gui`: Worker GUI binary

### Windows Build (`windows-build.yml`)

Builds Windows executables for backend, worker client, and worker GUI.

**Triggers:**
- Push to `main`/`master` branch (when C++ files change)
- Pull requests (when C++ files change)
- Manual workflow dispatch

**Jobs:**
1. **build-backend**: Builds C++ backend server
2. **build-worker**: Builds CLI worker client
3. **build-worker-gui**: Builds Qt-based GUI worker client

**Features:**
- Uses Visual Studio 2022
- Installs CMake and OpenSSL via Chocolatey
- Installs Qt 6.5.0 for GUI builds
- Builds Release configuration
- Uploads executables as artifacts

**Artifacts:**
- `windows-backend`: Backend server executable
- `windows-worker`: Worker client executable
- `windows-worker-gui`: Worker GUI executable

### Build All (`build-all.yml`)

Orchestrates builds for all platforms when a version tag is pushed.

**Triggers:**
- Push of version tag (e.g., `v1.0.0`)
- Manual workflow dispatch

## Usage

### Building Locally

**Android:**
```bash
cd android
./gradlew assembleRelease
```

**Linux:**
```bash
cd backend && mkdir build && cd build && cmake .. && make
cd ../../worker_client && mkdir build && cd build && cmake .. && make
cd ../../worker_client_gui && mkdir build && cd build && cmake .. && make
```

**Windows:**
```cmd
cd backend
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Triggering Workflows

**Automatic:**
- Push to main branch (triggers relevant workflows)
- Create a pull request (triggers relevant workflows)
- Push a version tag: `git tag v1.0.0 && git push origin v1.0.0`

**Manual:**
1. Go to GitHub Actions tab
2. Select workflow
3. Click "Run workflow"

### Downloading Artifacts

1. Go to the workflow run
2. Scroll to "Artifacts" section
3. Download the desired artifact

### Creating Releases

When you push a version tag:
```bash
git tag v1.0.0
git push origin v1.0.0
```

The Android workflow will automatically:
- Build the APK
- Create a GitHub release
- Attach the APK to the release

## Workflow Status

View workflow status at: `https://github.com/qaliblog/jobs/actions`

## Troubleshooting

### Android Build Fails
- Check JDK version (should be 17)
- Verify Android SDK is properly configured
- Check Gradle wrapper version

### Linux Build Fails
- Verify CMake version (3.20+)
- Check OpenSSL installation
- For GUI: Verify Qt installation

### Windows Build Fails
- Check Visual Studio 2022 installation
- Verify CMake is in PATH
- Check Qt installation path

### Artifacts Not Uploading
- Check file paths match expected locations
- Verify build completed successfully
- Check artifact retention settings

