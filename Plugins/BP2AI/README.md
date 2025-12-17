# BP2AI - Unreal Engine Blueprint to Markdown Exporter

BP2AI is an Unreal Engine 5 plugin designed to extract Blueprint logic and structure into human-readable and AI-friendly Markdown documentation. It helps in documenting complex Blueprint systems and providing context for AI coding assistants.

## Features

*   **Blueprint to Markdown**: Converts Blueprint graphs, functions, variables, and logic into structured Markdown.
*   **Execution Flow Analysis**: Analyzes and visualizes the execution flow of Blueprint graphs.
*   **Batch Export**: Export multiple Blueprints or entire folders from the Content Browser.
*   **Wide Support**: Supports standard Blueprints, Blueprint Interfaces, and Widget Blueprints.
*   **Configurable**: Customize export verbosity and formatting via C++ configuration.

## Installation

1.  Copy the `BP2AI` folder into your project's `Plugins` directory (e.g., `YourProject/Plugins/BP2AI`).
2.  Regenerate project files.
3.  Build your project.
4.  Enable the plugin in the Unreal Editor (Edit > Plugins > BP2AI).

## Usage

### 1. Blueprint Editor Toolbar
When you have a Blueprint open in the editor, click the **BP2AI** button in the toolbar.
*   This will open the **Blueprint Execution Flow** window.
*   It analyzes the current Blueprint and displays the execution flow and logic in a Markdown-friendly format.

### 2. Content Browser (Single Asset)
Right-click on any Blueprint asset in the Content Browser.
*   Select **Export Blueprint (BP2AI)** from the context menu.
*   The plugin will generate a Markdown file alongside the asset or in a designated export folder.

### 3. Content Browser (Batch Export)
Right-click on any folder in the Content Browser.
*   Select **Export Blueprints (BP2AI)** (or similar option in the context menu).
*   This will recursively find all Blueprints in the folder and export them to Markdown.

## Configuration

Configuration is currently handled via a C++ header file. To change settings, modify `Source/BP2AI/Public/Settings/BP2AIExportConfig.h` and recompile the plugin.

**Available Settings:**

*   `bPreviewEnabled`: Enable/disable execution flow log preview.
*   `PreviewMaxBlocks`: Limit the number of blocks shown in the preview.
*   `bShowDefaultParams`: Show default parameter values in function calls.
*   `bSeparateUserGraphs`: Separate user-defined graphs from the main event graph.
*   `bDetailedBlueprintLog`: Enable detailed logging for debugging.
*   `bSilenceInternalCategoriesDuringBatchExport`: Silence internal logs during batch operations to improve performance and reduce noise.

## Output Format

The plugin generates Markdown files containing:
*   Class hierarchy and interfaces.
*   Variables and properties.
*   Functions and their signatures.
*   Event graphs and execution flow logic.

---
*Copyright (c) 2025 A-Maze Games*
