## qlith-mini

This is a simple browser that uses Qt5 and litehtml. It works. 

$ ./qlith-mini/runme.sh

```
Attribute Qt::AA_EnableHighDpiScaling must be set before QCoreApplication is created.
qt.qpa.fonts: Populating font family aliases took 1273 ms. Replace uses of missing font family "Roboto Flex" with one that exists to avoid this cost. 
```

Otherwise it works well. 

===

## qlith-pro

This is a more complex browser that uses Qt5 and litehtml. It crashes, and I want to fix that. I absolutely need it to work using the current tech stack, not Qt6 and not stuff like QWebEngine! 

$ ./qlith-pro/runme.sh

This effectively runs

$ "./qlith-pro/build/qlith-pro.app/Contents/MacOS/qlith-pro" "./test_files/simple.html"


```
Running in debug mode
Current directory: "/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro"
Application directory: "/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build/qlith-pro.app/Contents/MacOS"
Current directory: "/Users/adam/Developer/vcs/github.twardoch/pub/qlith"
Application directory: "/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build/qlith-pro.app/Contents/MacOS"
Adding font family  "Cousine"  for font  "Cousine"
Adding font: "Cousine"  font family: "Cousine"
MainWindow: Setting up normal mode UI.
container_qt5: Using preferred font: "Arial"
container_qt5: Initialized with default font: "Arial" size: 16
container_qt5: Initialized with default font: Arial size: 16
MainWindow: Loading file from command line: "/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../test_files/simple.html"
loadUrl: Loading URL: "file:///Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../test_files/simple.html"
Loading URL: "file:///Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../test_files/simple.html"
loadHtml: Loading HTML with base URL: "file:///Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../test_files/simple.html"
container_qt5::set_base_url - Setting base URL to: "file:///Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../test_files/simple.html"
loadHtml: HTML document length: 178
loadHtml: Creating HTML document...
container_qt5::create_element - Returning nullptr (Diagnostic)
container_qt5::create_element - Returning nullptr (Diagnostic)
container_qt5::create_element - Returning nullptr (Diagnostic)
container_qt5::create_element - Returning nullptr (Diagnostic)
container_qt5::create_element - Returning nullptr (Diagnostic)
container_qt5::create_element - Returning nullptr (Diagnostic)
./qlith-pro/runme.sh: line 18: 39057 Segmentation fault: 11  "$dir/build/qlith-pro.app/Contents/MacOS/qlith-pro" "$dir/../test_files/simple.html"
```

### Diagnosis

This is a classic segmentation fault due to a stack overflow, likely caused by runaway recursion. The crash log for `qlith-pro` provides very strong clues. Here's an extensive plan to approach fixing this issue:

#### Debugging Plan for qlith-pro Crash

The core issue appears to be a stack overflow occurring within the `litehtml` library, specifically during CSS parsing, as evidenced by the repeated calls to `litehtml::parse_name_color` and the crash in `litehtml::css_tokenizer::consume_ident_like_token()` with a `KERN_PROTECTION_FAILURE` on the stack guard. The messages `container_qt5::create_element - Returning nullptr (Diagnostic)` are also highly suspicious and likely related.

Here's a multi-phase approach:

#### Phase 1: Deep Analysis and Information Gathering

1.  **Deconstruct the Crash Log:**

      * **Primary Suspect: Stack Overflow via Recursion:** The exception `EXC_BAD_ACCESS (SIGSEGV)` with `KERN_PROTECTION_FAILURE` at an address (`0x00007ff7bb36efd8`) right at the edge of the thread's stack guard is a tell-tale sign of a stack overflow.
      * **Recursive Culprit: `litehtml::parse_name_color`:** The stack trace shows hundreds of calls to `litehtml::parse_name_color` leading into `litehtml::css_tokenizer::consume_ident_like_token()`. This strongly suggests that `parse_name_color` is either directly or indirectly calling itself (or functions that call it back) without a proper termination condition when processing a specific CSS color value from `simple.html`.
      * **Symptom: `container_qt5::create_element - Returning nullptr`:** These messages indicate that your Qt-specific implementation of `litehtml::document_container` is failing to create HTML elements *before* the CSS parsing goes haywire. While this might not be the direct cause of the stack overflow in `parse_name_color`, `litehtml` might react poorly (e.g., by trying to apply styles to non-existent elements or their contexts) if the document structure it expects is incomplete or malformed due to these null elements. This could indirectly trigger the CSS parsing bug.

2.  **Forensic Examination of `test_files/simple.html`:**

      * This file is critical as it's the direct trigger.
      * **CSS Content:** Scrutinize all CSS within this file:
          * Inline styles (`style="..."`).
          * `<style>` blocks.
          * Links to external stylesheets (`<link rel="stylesheet" ...>`).
      * **Color Definitions:** Pay extremely close attention to how colors are defined. Are there any:
          * Unusual or very long named colors?
          * Custom CSS properties (CSS variables) used in color contexts that might be misinterpreted?
          * Potentially malformed color values (e.g., `rgb()`, `hsl()`, hex codes) that could confuse `parse_name_color`?
      * **Simplification Strategy:** Incrementally simplify `simple.html`:
        1.  Remove all CSS to see if the crash stops. If it does, CSS is the direct area.
        2.  If CSS is implicated, add back CSS rules/sections one by one (or remove them one by one from the original) until the crash reoccurs. Focus on rules involving colors first.
        3.  If removing all CSS doesn't stop the crash (less likely given the stack trace), then simplify the HTML structure itself.

3.  **Comparative Analysis: `qlith-pro` vs. `qlith-mini`:**

      * Since `qlith-mini` works, it's your golden reference.
      * **`litehtml::document_container` Implementation:** This is the bridge between Qt and `litehtml`.
          * Thoroughly compare your `container_qt5` (or similarly named) class in `qlith-pro` with its counterpart in `qlith-mini`.
          * **`create_element(...)`:** Why is it returning `nullptr` in `qlith-pro`? What are the arguments (tag name) when it returns `nullptr`? How does `qlith-mini`'s version behave for the same tags? Does `litehtml` properly handle `nullptr` returned from this function when it later tries to apply styles?
          * Other `document_container` methods: Compare implementations for `get_default_font_name`, `get_default_font_size`, methods related to resolving colors, fetching resources, etc.
      * **`litehtml` Library Version & Build:**
          * Are both projects using the *exact same version and commit* of the `litehtml` library? Check your `CMakeLists.txt` or build scripts. Even minor differences could matter.
          * How is `litehtml` built and linked in both projects?
      * **Initialization and Default Styles:**
          * How are default CSS styles or user-agent stylesheets applied in both?
          * The `qlith-pro` log shows `container_qt5: Using preferred font: "Arial"` and `Initialized with default font: "Arial" size: 16`. How does this compare to `qlith-mini`?

#### Phase 2: Active Debugging and Isolation

1.  **Leverage the Debugger (LLDB on macOS):**

      * You are already running in debug mode. Launch `qlith-pro` with `simple.html` under LLDB.
      * **Key Breakpoints:**
          * `litehtml::parse_name_color`: When hit, inspect its arguments, especially the `css_token const& token` (or similar). What is its text value? How does this value change in successive calls leading to the stack overflow?
          * `litehtml::css_tokenizer::consume_ident_like_token`: Inspect the input stream or string it's operating on.
          * `container_qt5::create_element` (in your `qlith-pro` code): Inspect the tag name being requested. Why does it return `nullptr`?
      * **Call Stack Analysis:** When a breakpoint (especially in `parse_name_color`) is hit multiple times, examine the full call stack. Look for the pattern of recursion.
      * **Conditional Breakpoints:** If `parse_name_color` is called legitimately many times, set a conditional breakpoint to trigger only after a high number of calls, or if a specific problematic token value appears.

2.  **Isolate the Problematic Code Path in `litehtml` (if necessary):**

      * If the issue seems to be within `litehtml` itself (triggered by a specific valid-but-problematic CSS construct), try to create an even more minimal C++ test case that uses only the `litehtml` library to parse the identified problematic CSS snippet, without involving Qt. This helps determine if the bug is purely in `litehtml` or in its interaction with your `document_container`.

3.  **Code Review (Targeted):**

      * **`litehtml`:** If you identify a suspect token or CSS structure, examine the source code (possibly from `external.txt` or the `litehtml` repository) of `parse_name_color` and related CSS tokenizing/parsing functions. Look for logic flaws that could lead to unterminated recursion or loops (e.g., not consuming input, incorrect state transitions).
      * **`qlith-pro`'s `document_container`:** Focus on `create_element`. Is there a missing case for a tag? Is it correctly allocating and returning element objects?

#### Phase 3: Solution Development and Verification

1.  **Formulate and Test Hypotheses:** Based on your debugging:

      * **Hypothesis 1: Malformed/Problematic CSS in `simple.html`:** The fix is to correct the CSS.
      * **Hypothesis 2: Bug in `litehtml`:**
          * Can the CSS be rewritten to avoid the bug? (Workaround)
          * Can `litehtml` be patched? (Check its GitHub issues/PRs, or develop a patch). The issue likely lies in how it parses an identifier or a specific sequence of tokens within a color context.
      * **Hypothesis 3: Bug in `qlith-pro`'s `document_container` (`create_element` returning `nullptr`):** If `litehtml` receives `nullptr` for an element it expects to exist, its subsequent attempts to apply styles (like colors) could fail spectacularly, potentially by trying to dereference a null pointer or by getting into an inconsistent state that leads the CSS parser astray. Fixing `create_element` to correctly return valid element objects would be the solution.

2.  **Implement and Test Fixes:**

      * Apply the most likely fix first.
      * Test *immediately* with `simple.html`.
      * If the crash is resolved, test with a broader range of HTML/CSS files, including complex ones and edge cases, to ensure no regressions were introduced.
      * Test `qlith-mini` with the same `simple.html` to see how it behaves (it should work, and its console log might show how `create_element` handles the elements from `simple.html` correctly).

3.  **Consider the `create_element` / `parse_name_color` Link:**
    It's plausible that `litehtml`, after failing to create an element (getting `nullptr` from your `create_element`), still proceeds to try and parse/apply styles for it or its children. When it comes to a style like `color: somevalue;`, if the context (the element it's trying to apply to) is null or invalid, `parse_name_color` might be called with uninitialized or garbage data, or it might lack necessary contextual information normally provided by a valid element, leading to the parser going off the rails.

By following this structured approach, focusing on the clues from the crash log and systematically isolating the issue, you should be able to pinpoint the root cause and implement a solution. The recursion in `parse_name_color` is your strongest lead for the direct cause of the stack overflow. The `nullptr` from `create_element` is a very strong candidate for an underlying issue that might trigger this parsing problem.

### Crash log

/Users/adam/Library/Logs/DiagnosticReports/qlith-pro-2025-05-11-152643.ips 

### Implemented Fixes

The segfault in qlith-pro has been addressed by implementing the following critical fixes:

1. **Fixed Container Element Implementation:**
   - Modified `container_qt5::create_element` to properly handle tag creation and avoid incorrect nullptr returns
   - Added enhanced debugging information to track element creation and attributes
   - Fixed HTML tag handling approach to match the working implementation in qlith-mini

2. **Fixed Color Handling:**
   - Enhanced `container_qt5::resolve_color` to properly handle CSS values that aren't actually colors
   - Added filtering for decoration properties like "underline", "inherit", etc. to prevent recursion
   - Improved error handling to avoid passing invalid values up the chain

3. **Implemented Drawing Functions:**
   - Ported working drawing implementations from qlith-mini's `container_qpainter`
   - Added proper text rendering with font metrics and positioning
   - Implemented border drawing, background fills, and gradient support
   - Added clipping region support for CSS layout

### Current Status

The application now successfully renders HTML content, confirming that the core segfault issue has been resolved. However, there's still a secondary segmentation fault that occurs after several successful render frames, likely related to cleanup or memory management. This will be addressed in a future update.

The screenshot shows the application rendering the test HTML file correctly, with proper text layout and styling:

![qlith-pro rendering HTML content](qlith-pro-rendering.png)

Next steps will focus on resolving the remaining memory management issues and implementing more robust error handling throughout the rendering pipeline.
