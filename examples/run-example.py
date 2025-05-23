#!/usr/bin/env -S uv run -s
# /// script
# dependencies = ["fire", "rich"]
# ///
# this_file: examples/run-example.py

import os
import sys
import subprocess
import time
from pathlib import Path
from typing import List, Optional, Dict, Any, Union
import fire
from rich.console import Console
from rich.panel import Panel

# Initialize rich console
console = Console()


class TimeoutError(Exception):
    """Exception raised when a command times out."""

    pass


def run_command_with_timeout(cmd: List[str], timeout: int = 120) -> tuple:
    """Run a command with a timeout.

    Args:
        cmd: Command to run as a list of strings
        timeout: Timeout in seconds

    Returns:
        Tuple of (return_code, stdout, stderr)
    """
    process = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )

    try:
        stdout, stderr = process.communicate(timeout=timeout)
        return process.returncode, stdout, stderr
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
        return 124, "", f"Command timed out after {timeout} seconds"


def find_html_files(input_path: Path) -> List[Path]:
    """Find all HTML files in the given path.

    Args:
        input_path: Path to a file or directory

    Returns:
        List of HTML file paths
    """
    if input_path.is_file():
        if input_path.suffix.lower() == ".html":
            return [input_path]
        return []

    html_files = []
    for html_file in input_path.glob("**/*.html"):
        html_files.append(html_file)

    return sorted(html_files)


class QlithRunner:
    """Tool to generate images from HTML files using Qlith engines."""

    def __init__(self):
        # Get root directory
        self.script_dir = Path(__file__).parent.absolute()
        self.root_dir = self.script_dir.parent

        # Set environment variables
        os.environ["QT_QPA_PLATFORM"] = "offscreen"
        os.environ["QLITH_SKIP_DEFAULT_LOAD"] = "1"
        os.environ["QLITH_DEBUG"] = "1"
        os.environ["QT_SCALE_FACTOR"] = "1"  # Changed from 2 to 1 for proper rendering

    def get_engine_path(self, engine: str) -> Path:
        """Get the path to the engine executable.

        Args:
            engine: Engine name ('mini' or 'pro')

        Returns:
            Path to the engine executable
        """
        if engine == "mini":
            path = (
                self.root_dir
                / "qlith-mini"
                / "build"
                / "browser"
                / "qlith.app"
                / "Contents"
                / "MacOS"
                / "qlith"
            )
        else:  # pro
            path = (
                self.root_dir
                / "qlith-pro"
                / "build"
                / "qlith-pro.app"
                / "Contents"
                / "MacOS"
                / "qlith-pro"
            )

        if not path.is_file() or not os.access(path, os.X_OK):
            raise FileNotFoundError(
                f"Engine executable not found or not executable: {path}"
            )

        return path

    def create_output_directory(self, output_dir: Path) -> None:
        """Create output directory if it doesn't exist.

        Args:
            output_dir: Output directory path
        """
        output_dir.mkdir(parents=True, exist_ok=True)

    def generate_output(
        self,
        engine: str,
        engine_path: Path,
        html_file: Path,
        output_dir: Path,
        formats: List[str],
        width: int = 2048,
        height: int = 2048,
        timeout: int = 120,
    ) -> Dict[str, Any]:
        """Generate output for a given HTML file using the specified engine.

        Args:
            engine: Engine name ('mini' or 'pro')
            engine_path: Path to the engine executable
            html_file: Path to the HTML file
            output_dir: Output directory
            formats: List of output formats ('svg', 'png')
            width: Width in pixels for rendering
            height: Height in pixels for rendering
            timeout: Timeout in seconds

        Returns:
            Dictionary with results
        """
        results = {}
        basename = html_file.stem

        # Add engine prefix to the output filename
        name_with_engine = f"{basename}-{engine}"

        # Make sure we have absolute paths
        html_file = html_file.absolute()
        output_dir = output_dir.absolute()

        # Ensure output directory exists before starting
        output_dir.mkdir(parents=True, exist_ok=True)
        console.print(f"Ensuring output directory exists: {output_dir}")

        # For qlith-pro engine, set debug environment variables
        if engine == "pro":
            os.environ["QLITH_EXPORT_DEBUG"] = "1"
            os.environ["QLITH_VERBOSE"] = "1"

        for fmt in formats:
            output_file = output_dir / f"{name_with_engine}.{fmt}"

            # Print the absolute path of the output file that will be created
            console.print(f"  Processing with [bold]qlith-{engine}[/bold]...")
            console.print(f"    Generating [bold]{fmt.upper()}[/bold] to:")
            console.print(f"    [blue]{output_file}[/blue]")

            # Initialize tmp_output_file to avoid unbounded variable issues
            tmp_output_file = None

            # Initialize rel_output_file to avoid unbounded variable issues
            rel_output_file = None

            # For pro engine, try using the current directory for output
            if engine == "pro":
                # Try relative path to current directory
                rel_output_file = Path(f"./output-{name_with_engine}.{fmt}")
                console.print(
                    f"    Using current directory output for pro engine: {rel_output_file.absolute()}"
                )
                cmd = [
                    str(engine_path),
                    f"--{fmt}",
                    str(rel_output_file),
                    "--width",
                    str(width),
                    "--height",
                    str(height),
                    str(html_file),
                ]
            else:
                cmd = [
                    str(engine_path),
                    f"--{fmt}",
                    str(output_file),
                    "--width",
                    str(width),
                    "--height",
                    str(height),
                    str(html_file),
                ]

            # Print the full command being executed for debugging
            console.print(f"    Executing command: {' '.join(cmd)}")

            start_time = time.time()
            returncode, stdout, stderr = run_command_with_timeout(cmd, timeout)
            duration = time.time() - start_time

            # Print additional debug information for pro engine
            if engine == "pro":
                console.print(f"    Command output (stdout):\n{stdout}")
                console.print(f"    Command error output (stderr):\n{stderr}")

            if returncode == 124:
                console.print(
                    f"    [yellow]WARNING:[/yellow] qlith-{engine} {fmt.upper()} generation timed out after {timeout} seconds"
                )
                results[fmt] = {
                    "success": False,
                    "reason": "timeout",
                    "duration": duration,
                    "output_file": output_file,
                }
            elif returncode != 0:
                console.print(
                    f"    [red]ERROR:[/red] qlith-{engine} {fmt.upper()} generation failed with code {returncode}"
                )
                console.print(f"    stderr: {stderr}")
                results[fmt] = {
                    "success": False,
                    "reason": "error",
                    "duration": duration,
                    "stderr": stderr,
                    "output_file": output_file,
                }
            else:
                console.print(
                    f"    [green]qlith-{engine} {fmt.upper()} generation completed successfully[/green] in {duration:.2f}s"
                )

                # If we used a relative path for pro engine, move it to the correct location
                if engine == "pro" and rel_output_file and rel_output_file.exists():
                    console.print(
                        f"    Relative output file exists: {rel_output_file.absolute()} ({rel_output_file.stat().st_size} bytes)"
                    )
                    try:
                        # Copy the file to the intended destination
                        import shutil

                        shutil.copy2(rel_output_file, output_file)
                        console.print(
                            f"    [green]Copied file to:[/green] {output_file}"
                        )

                        # Delete the original file after successful copy
                        rel_output_file.unlink()
                        console.print(
                            f"    [green]Deleted original file:[/green] {rel_output_file.absolute()}"
                        )
                    except Exception as e:
                        console.print(f"    [red]Error copying file:[/red] {e}")

                results[fmt] = {
                    "success": True,
                    "duration": duration,
                    "output_file": output_file,
                }

            # Check if the output file actually exists
            if output_file.exists():
                console.print(
                    f"    [green]Verified:[/green] Output file exists ({output_file.stat().st_size} bytes)"
                )
            else:
                console.print(
                    f"    [red]WARNING:[/red] Output file does not exist: {output_file}"
                )

                # Look for files that might have been created elsewhere
                if engine == "pro":
                    # Search for recently created files with similar names
                    console.print("    Searching for similar output files...")
                    search_dirs = [
                        Path("."),
                        Path.home(),
                        self.script_dir,
                        self.root_dir,
                        self.root_dir / "qlith-pro",
                    ]

                    for search_dir in search_dirs:
                        if search_dir.exists():
                            potential_files = list(
                                search_dir.glob(f"*{basename}*.{fmt}")
                            )
                            if potential_files:
                                console.print(
                                    f"    [yellow]Found potential output files in {search_dir}:[/yellow]"
                                )
                                for f in potential_files:
                                    if f.is_file() and f.stat().st_mtime > start_time:
                                        console.print(
                                            f"      - {f} ({f.stat().st_size} bytes) [RECENT]"
                                        )
                                        try:
                                            # Copy the file to the intended destination
                                            import shutil

                                            shutil.copy2(f, output_file)
                                            console.print(
                                                f"    [green]Copied file to:[/green] {output_file}"
                                            )

                                            # Delete the original file after successful copy
                                            f.unlink()
                                            console.print(
                                                f"    [green]Deleted original file:[/green] {f.absolute()}"
                                            )
                                            break
                                        except Exception as e:
                                            console.print(
                                                f"    [red]Error copying file:[/red] {e}"
                                            )
                                    else:
                                        console.print(
                                            f"      - {f} ({f.stat().st_size} bytes)"
                                        )

        return results

    def create_test_file(self) -> Path:
        """Create a simple test HTML file.

        Returns:
            Path to the created test file
        """
        test_file = self.script_dir / "simple_test.html"
        with open(test_file, "w") as f:
            f.write("""<!DOCTYPE html>
<html>
<head>
    <title>Simple Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            max-width: 100%;
            overflow-x: hidden;
        }
        h1 {
            color: blue;
        }
        p {
            color: black;
        }
    </style>
</head>
<body>
    <h1>Hello World</h1>
    <p>This is a simple test page for Qlith rendering.</p>
</body>
</html>
""")

        console.print(f"Created simple test file: [bold]{test_file}[/bold]")
        return test_file

    def run(
        self,
        engine: Optional[str] = None,
        input: Optional[str] = None,
        output: Optional[str] = None,
        svg: bool = False,
        png: bool = False,
        width: int = 2048,
        height: int = 2048,
        timeout: int = 120,
        test_simple: bool = False,
    ) -> int:
        """Generate images from HTML files using Qlith engines.

        Args:
            engine: Engine to use (mini or pro, default: both)
            input: Input HTML file or directory (default: examples directory)
            output: Output directory (default: current directory + engine subfolder)
            svg: Generate SVG output
            png: Generate PNG output
            width: Width in pixels for rendering (default: 2048)
            height: Height in pixels for rendering (default: 2048)
            timeout: Timeout in seconds (default: 120)
            test_simple: Generate and render a simple test file only

        Returns:
            Exit code (0 for success, non-zero for failure)
        """
        # Set default input if not provided
        if input is None:
            input_path = self.root_dir / "examples"
        else:
            input_path = Path(input)

        # Set default formats if none specified
        formats = []
        if svg:
            formats.append("svg")
        if png:
            formats.append("png")
        if not formats:  # If no formats specified, use both
            formats = ["svg", "png"]

        # Set engines to use
        engines = []
        if engine:
            if engine not in ["mini", "pro"]:
                console.print(
                    f"[red]Error:[/red] Invalid engine '{engine}'. Must be 'mini' or 'pro'."
                )
                return 1
            engines.append(engine)
        else:
            engines = ["mini", "pro"]

        console.print(
            Panel.fit(
                f"[bold]Qlith Renderer[/bold]\n"
                f"Engines: {', '.join(engines)}\n"
                f"Formats: {', '.join(formats)}\n"
                f"Input: {input_path}\n"
                + (
                    f"Output: {output}"
                    if output
                    else "Output: <engine-specific folder>"
                )
                + f"\nRendering dimensions: {width}x{height} px",
                title="Configuration",
                border_style="blue",
            )
        )

        # Find engine paths
        engine_paths = {}
        for eng in engines:
            try:
                engine_paths[eng] = self.get_engine_path(eng)
                console.print(
                    f"Found {eng} engine at: [bold]{engine_paths[eng]}[/bold]"
                )
            except FileNotFoundError as e:
                console.print(f"[red]ERROR:[/red] {e}")
                console.print(f"Please build the qlith-{eng} application first")
                return 1

        # Process test simple case
        if test_simple:
            console.print("\n[bold]Creating and rendering a simple test file...[/bold]")
            test_file = self.create_test_file()

            # Process the test file with each engine
            for eng in engines:
                output_dir = Path(output) if output else (self.script_dir / eng)
                output_dir = output_dir.absolute()  # Ensure absolute path
                console.print(f"Output directory (absolute): [bold]{output_dir}[/bold]")
                self.create_output_directory(output_dir)

                results = self.generate_output(
                    eng,
                    engine_paths[eng],
                    test_file,
                    output_dir,
                    formats,
                    width,
                    height,
                    timeout,
                )

            console.print("[bold green]Done with simple test[/bold green]")
            return 0

        # Find HTML files to process
        html_files = find_html_files(input_path)

        if not html_files:
            console.print(f"[red]No HTML files found in {input_path}[/red]")
            return 1

        console.print(f"Found [bold]{len(html_files)}[/bold] HTML files to process")

        # Process each HTML file with each engine
        for eng in engines:
            output_dir = Path(output) if output else (self.script_dir / eng)
            output_dir = output_dir.absolute()  # Ensure absolute path
            self.create_output_directory(output_dir)

            console.print(f"\nUsing engine: [bold]{eng}[/bold]")
            console.print(f"Output directory: [bold]{output_dir}[/bold]")

            for html_file in html_files:
                console.print(
                    Panel(
                        f"Processing [bold]{html_file.name}[/bold]",
                        border_style="green",
                    )
                )

                results = self.generate_output(
                    eng,
                    engine_paths[eng],
                    html_file,
                    output_dir,
                    formats,
                    width,
                    height,
                    timeout,
                )

                console.print(f"Done with [bold]{html_file.name}[/bold]")

                # Check if all output files exist
                success = all(
                    r.get("success", False) and Path(r.get("output_file", "")).exists()
                    for r in results.values()
                )
                if not success:
                    console.print(
                        f"[red]WARNING:[/red] Some output files for {html_file.name} were not created correctly"
                    )

        console.print(
            Panel.fit(
                "[bold green]Processing complete![/bold green]", border_style="green"
            )
        )

        # Final verification of output directories and files
        for eng in engines:
            output_dir = Path(output) if output else (self.script_dir / eng)
            output_dir = output_dir.absolute()
            console.print(
                f"Output files for qlith-{eng} are in [bold]{output_dir}[/bold]"
            )

            # List files in output directory
            if output_dir.exists():
                files = list(output_dir.glob("*"))
                if files:
                    console.print(
                        f"[green]Found {len(files)} files in output directory:[/green]"
                    )
                    for f in files:
                        console.print(f"  - {f.name} ({f.stat().st_size} bytes)")
                else:
                    console.print(
                        f"[yellow]Warning: No files found in output directory.[/yellow]"
                    )
            else:
                console.print(f"[red]ERROR: Output directory does not exist![/red]")

        return 0

    def all(self) -> int:
        """Run with all engines and formats on the default examples directory.

        Returns:
            Exit code (0 for success, non-zero for failure)
        """
        return self.run(
            engine=None,  # Use both engines
            input=str(self.root_dir / "examples"),
            output=None,  # Use default engine-specific folders
            svg=True,
            png=True,
            width=2048,
            height=2048,
            timeout=120,
        )

    def test(self) -> int:
        """Run a quick test with both engines using a simple test file.

        Returns:
            Exit code (0 for success, non-zero for failure)
        """
        return self.run(
            engine=None,  # Use both engines
            test_simple=True,
            svg=True,
            png=True,
            width=2048,
            height=2048,
        )


def main():
    """Main entry point for the CLI tool."""
    return fire.Fire(QlithRunner())


if __name__ == "__main__":
    sys.exit(main())
