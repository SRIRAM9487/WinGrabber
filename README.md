# WindGrabber

**WindGrabber** is a high-performance desktop application designed to index and manage files across Windows drives. Built with **C++**, **Qt**, and **SQLite**, it offers lightning-fast file searches and efficient system resource usage, addressing the limitations of native Windows search.

---

## Features

- **Full Disk Indexing**: Scans and indexes files across all drives for quick access.  
- **Efficient Metadata Storage**: Uses SQLite to store file metadata (name, path, hashes).  
- **Background Processing**: Incremental hashing and updates track file changes without impacting performance.  
- **Optimized Performance**: Multi-threaded hashing and batch database insertion for handling large file volumes.  
- **User-Friendly Interface**: Intuitive Qt-based GUI for seamless file search and management.  

---

## Tech Stack

- **C++**: Core application logic and performance-critical operations  
- **Qt**: Cross-platform framework for building the graphical user interface  
- **SQLite**: Lightweight database for efficient storage and retrieval of file metadata  

---

## Installation

### Prerequisites
- Windows 10 or later  
- Qt 5.x or 6.x  
- SQLite3  
- C++ compiler (e.g., MSVC, MinGW)  

### Clone the Repository
```bash
git clone https://github.com/SRIRAM9487/WinGrabber.git
cd WinGrabber
use QT to build the app and run it...
