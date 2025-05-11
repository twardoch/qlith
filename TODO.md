# TODO

## qlith-mini

This is a simple browser that uses Qt5 and litehtml. It works. 

$ ./qlith-mini/runme.sh

```
Attribute Qt::AA_EnableHighDpiScaling must be set before QCoreApplication is created.
qt.qpa.fonts: Populating font family aliases took 1273 ms. Replace uses of missing font family "Roboto Flex" with one that exists to avoid this cost. 
```

Otherwise it works well. 

## qlith-pro

This is a more complex browser that uses Qt5 and litehtml. It crashes, and I want to fix that. I absolutely need it to work using the current tech stack, not Qt6 and not stuff like QWebEngine! 

$ ./qlith-pro/runme.sh

```
failed to open font file, path =  ":/res/font/Cousine-Regular.ttf"
failed to open font file, path =  ":/res/font/arialuni.ttf"
failed to open font file, path =  ":/res/font/DroidSans.ttf"
failed to open font file, path =  ":/res/font/fa-regular-400.ttf"
./qlith-pro/runme.sh: line 14: 13470 Segmentation fault: 11  "$dir/build/qlith-pro.app/Contents/MacOS/qlith-pro" "$dir/../test_files/fl8.html"
```

TODO:

- [ ] Fix the segfault.

