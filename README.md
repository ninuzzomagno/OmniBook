# OmniBook
## A book reader app for PSVITA with traslation support

### Introduction
OmniBook is an app for PSVITA written in C++. It uses the main following library:
- **MuPDF**: so it can read PDF, EPUB, MOBI, CBZ, CBR e XPS
- **SDL2**: for graphics and input
- **ImGui**: for widgets and windows
- **fmt**: for *std::format* support

### Building
If you want to build this app from source code, you need to extract the libmupdf.a from its tz archive. 

### Features
The **traslation support** is acheived through *SceHttp* and *Google traslate*. You can traslate a single word or the whole page. Supported languages are: Italian, English, Spanish, French and German.
Everything in this app works with front touch.
There is a **night mode**
