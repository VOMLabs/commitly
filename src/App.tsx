import { useState, useEffect, useCallback } from "react";
import {
  GitBranch,
  Database,
  RefreshCcw,
  Sparkles,
  User,
  Square,
  Plus,
  Minus,
  Settings,
  Maximize2,
  CheckSquare,
  Clock,
  Archive,
  Users,
  FileText,
  Search,
  GitCommit,
  Upload,
  Download,
  ChevronRight,
  Minus as MinusIcon,
  X,
  Square as SquareIcon,
} from "lucide-react";
import { useHotkey } from "@tanstack/react-hotkeys";
import { getCurrentWindow } from "@tauri-apps/api/window";
import "./App.css";

interface FileItem {
  name: string;
  status: "added" | "modified" | "deleted" | "untracked";
}

interface CommitItem {
  hash: string;
  summary: string;
  author: string;
  date: string;
  files: string[];
}

const mockFiles: FileItem[] = [
  { name: ".gitignore", status: "modified" },
  { name: "src/App.tsx", status: "modified" },
  { name: "src/main.tsx", status: "modified" },
  { name: "package.json", status: "modified" },
  { name: "vite.config.ts", status: "modified" },
  { name: "tsconfig.json", status: "modified" },
  { name: "README.md", status: "added" },
  { name: "src/utils/api.ts", status: "untracked" },
  { name: "src/styles/global.css", status: "modified" },
  { name: ".env.local", status: "untracked" },
];

const mockCommits: CommitItem[] = [
  { hash: "a1b2c3d", summary: "feat: add dark mode theming", author: "mateo", date: "2 hours ago", files: ["src/App.css", "src/App.tsx"] },
  { hash: "e4f5g6h", summary: "fix: resolve build errors", author: "mateo", date: "4 hours ago", files: ["vite.config.ts"] },
  { hash: "i7j8k9l", summary: "feat: implement keyboard navigation", author: "mateo", date: "6 hours ago", files: ["src/App.tsx"] },
  { hash: "m0n1o2p", summary: "chore: update dependencies", author: "mateo", date: "1 day ago", files: ["package.json"] },
  { hash: "q3r4s5t", summary: "docs: add README content", author: "mateo", date: "1 day ago", files: ["README.md"] },
  { hash: "u6v7w8x", summary: "refactor: restructure components", author: "mateo", date: "2 days ago", files: ["src/App.tsx", "src/main.tsx"] },
  { hash: "y9z0a1b", summary: "feat: initial commit", author: "mateo", date: "3 days ago", files: ["package.json", "vite.config.ts", "tsconfig.json"] },
];

type DiffLine = { type: "header"; content: string } | { type: "added" | "removed"; num?: number; content: string };

const mockDiff: DiffLine[] = [
  { type: "header", content: "@@ -0,0 +1,5 @@" },
  { type: "added", num: 1, content: "No preview available" },
  { type: "added", num: 2, content: "" },
  { type: "added", num: 3, content: "This file has no staged changes." },
  { type: "added", num: 4, content: "Select a different file to view its diff." },
  { type: "added", num: 5, content: "" },
];

const fileDiffs: Record<string, DiffLine[]> = {
  ".gitignore": [
    { type: "header", content: "@@ -0,0 +1,8 @@" },
    { type: "added", num: 1, content: "node_modules/" },
    { type: "added", num: 2, content: "dist/" },
    { type: "added", num: 3, content: ".env.local" },
    { type: "added", num: 4, content: ".env.*.local" },
    { type: "added", num: 5, content: "*.log" },
    { type: "added", num: 6, content: "npm-debug.log*" },
    { type: "added", num: 7, content: ".DS_Store" },
    { type: "added", num: 8, content: "coverage/" },
  ],
  "src/App.tsx": [
    { type: "header", content: "@@ -1,12 +1,28 @@" },
    { type: "added", num: 1, content: "import { useState } from 'react';" },
    { type: "added", num: 2, content: "import {" },
    { type: "added", num: 3, content: "  GitBranch," },
    { type: "added", num: 4, content: "  Database," },
    { type: "added", num: 5, content: "  Sparkles," },
    { type: "added", num: 6, content: "} from 'lucide-react';" },
    { type: "added", num: 7, content: "function App() {" },
    { type: "added", num: 8, content: "  const [count, setCount] = useState(0);" },
    { type: "added", num: 9, content: "  return (" },
    { type: "added", num: 10, content: "    <div>" },
  ],
  "package.json": [
    { type: "header", content: "@@ -1,21 +1,26 @@" },
    { type: "added", num: 1, content: "{" },
    { type: "added", num: 2, content: '  "name": "commitly",' },
    { type: "added", num: 3, content: '  "version": "0.1.0",' },
    { type: "added", num: 4, content: '  "type": "module",' },
    { type: "added", num: 5, content: '  "scripts": {' },
    { type: "added", num: 6, content: '    "dev": "vite",' },
    { type: "added", num: 7, content: '    "build": "tsc && vite build",' },
  ],
  "README.md": [
    { type: "header", content: "@@ -0,0 +1,15 @@" },
    { type: "added", num: 1, content: "# Commitly" },
    { type: "added", num: 2, content: "" },
    { type: "added", num: 3, content: "A modern Git desktop client built with Tauri and React." },
    { type: "added", num: 4, content: "" },
    { type: "added", num: 5, content: "## Getting Started" },
    { type: "added", num: 6, content: "npm install && npm run tauri dev" },
  ],
  "src/styles/global.css": [
    { type: "header", content: "@@ -1,4 +1,12 @@" },
    { type: "added", num: 1, content: "* {" },
    { type: "added", num: 2, content: "  box-sizing: border-box;" },
    { type: "added", num: 3, content: "  margin: 0;" },
    { type: "added", num: 4, content: "  padding: 0;" },
  ],
  "vite.config.ts": [
    { type: "header", content: "@@ -1,8 +1,12 @@" },
    { type: "added", num: 1, content: "import { defineConfig } from 'vite';" },
    { type: "added", num: 2, content: "import react from '@vitejs/plugin-react';" },
    { type: "added", num: 3, content: "import tailwindcss from '@tailwindcss/vite';" },
    { type: "added", num: 4, content: "export default defineConfig(async () => ({" },
    { type: "added", num: 5, content: "  plugins: [react(), tailwindcss()]," },
  ],
  "tsconfig.json": [
    { type: "header", content: "@@ -1,6 +1,13 @@" },
    { type: "added", num: 1, content: "{" },
    { type: "added", num: 2, content: "  \"compilerOptions\": {" },
    { type: "added", num: 3, content: "    \"target\": \"ES2020\"," },
    { type: "added", num: 4, content: "    \"lib\": [\"ES2020\", \"DOM\"]," },
  ],
  "src/main.tsx": [
    { type: "header", content: "@@ -3,10 +3,12 @@" },
    { type: "added", num: 3, content: "import App from './App';" },
    { type: "added", num: 4, content: "import './index.css';" },
    { type: "added", num: 5, content: "createRoot(document.getElementById('root')!).render(" },
  ],
  "src/utils/api.ts": [
    { type: "header", content: "@@ -0,0 +1,18 @@" },
    { type: "added", num: 1, content: "const API_BASE = '/api';" },
    { type: "added", num: 2, content: "export async function fetchRepo() {" },
  ],
  ".env.local": [
    { type: "header", content: "@@ -0,0 +1,5 @@" },
    { type: "added", num: 1, content: "VITE_API_URL=http://localhost:3000" },
    { type: "added", num: 2, content: "VITE_WS_URL=ws://localhost:3001" },
  ],
};

const commitDiffs: Record<string, DiffLine[]> = {
  "a1b2c3d": [
    { type: "header", content: "@@ -1,5 +1,20 @@" },
    { type: "added", num: 1, content: ":root {" },
    { type: "added", num: 2, content: "  --color-bg: #010101;" },
    { type: "added", num: 3, content: "  --color-sidebar: #121212;" },
    { type: "added", num: 4, content: "  --color-border: #1a1a1a;" },
    { type: "added", num: 5, content: "  --color-primary: #1f6feb;" },
  ],
  "e4f5g6h": [
    { type: "header", content: "@@ -1,3 +1,8 @@" },
    { type: "added", num: 1, content: "export default defineConfig(async () => ({" },
    { type: "added", num: 2, content: "  plugins: [react(), tailwindcss()]," },
    { type: "added", num: 3, content: "  server: {" },
  ],
  "i7j8k9l": [
    { type: "header", content: "@@ -1,10 +1,25 @@" },
    { type: "added", num: 1, content: "import { useHotkey } from '@tanstack/react-hotkeys';" },
    { type: "added", num: 2, content: "useHotkey('ArrowDown', () => {" },
    { type: "added", num: 3, content: "  navigateToFile(selectedIndex + 1);" },
  ],
};

function App() {
  const [activeTab, setActiveTab] = useState<"changes" | "history">("changes");
  const [selectedFile, setSelectedFile] = useState(".gitignore");
  const [selectedIndex, setSelectedIndex] = useState(0);
  const [currentDiff, setCurrentDiff] = useState<DiffLine[]>(fileDiffs[".gitignore"] || mockDiff);
  const [checkedFiles, setCheckedFiles] = useState<Set<string>>(new Set(mockFiles.map((f) => f.name)));
  const [summary, setSummary] = useState("");
  const [description, setDescription] = useState("");
  const [isGeneratingSummary, setIsGeneratingSummary] = useState(false);
  const [isGeneratingDesc, setIsGeneratingDesc] = useState(false);
  const [lastFetched] = useState("2 minutes ago");
  
  const [historySearch, setHistorySearch] = useState("");
  const [selectedCommit, setSelectedCommit] = useState<CommitItem | null>(null);
  const [historyCommitDiff, setHistoryCommitDiff] = useState<DiffLine[]>([]);

  const appWindow = getCurrentWindow();

  const filteredCommits = mockCommits.filter(c => 
    c.summary.toLowerCase().includes(historySearch.toLowerCase()) ||
    c.hash.includes(historySearch)
  );

  const navigateToFile = useCallback((index: number) => {
    const clampedIndex = Math.max(0, Math.min(index, mockFiles.length - 1));
    setSelectedIndex(clampedIndex);
    setSelectedFile(mockFiles[clampedIndex].name);
  }, []);

  useHotkey("ArrowDown", () => {
    if (activeTab === "changes") {
      navigateToFile(selectedIndex + 1);
    }
  });

  useHotkey("ArrowUp", () => {
    if (activeTab === "changes") {
      navigateToFile(selectedIndex - 1);
    }
  });

  useHotkey({ key: "ArrowRight", ctrl: true }, () => {
    setActiveTab("history");
  });

  useHotkey({ key: "ArrowLeft", ctrl: true }, () => {
    setActiveTab("changes");
  });

  useEffect(() => {
    const diff = fileDiffs[selectedFile];
    if (diff) {
      setCurrentDiff(diff);
    } else {
      setCurrentDiff(mockDiff);
    }
    const idx = mockFiles.findIndex(f => f.name === selectedFile);
    if (idx !== -1) {
      setSelectedIndex(idx);
    }
  }, [selectedFile]);

  useEffect(() => {
    if (selectedCommit) {
      const diff = commitDiffs[selectedCommit.hash];
      if (diff) {
        setHistoryCommitDiff(diff);
      } else {
        setHistoryCommitDiff([
          { type: "header", content: `@@ ${selectedCommit.hash[0]}${selectedCommit.hash[1]}..${selectedCommit.hash[0]}${selectedCommit.hash[1]} @@` },
          { type: "added", num: 1, content: `${selectedCommit.summary}` },
          { type: "added", num: 2, content: "" },
        ]);
      }
    }
  }, [selectedCommit]);

  const toggleFile = (fileName: string) => {
    const newChecked = new Set(checkedFiles);
    if (newChecked.has(fileName)) {
      newChecked.delete(fileName);
    } else {
      newChecked.add(fileName);
    }
    setCheckedFiles(newChecked);
  };

  const toggleAll = () => {
    if (checkedFiles.size === mockFiles.length) {
      setCheckedFiles(new Set());
    } else {
      setCheckedFiles(new Set(mockFiles.map((f) => f.name)));
    }
  };

  const generateSummary = () => {
    setIsGeneratingSummary(true);
    setTimeout(() => {
      setSummary("feat: add gitignore patterns for node_modules");
      setIsGeneratingSummary(false);
    }, 1500);
  };

  const generateDescription = () => {
    setIsGeneratingDesc(true);
    setTimeout(() => {
      setDescription("Added node_modules and environment files to avoid committing sensitive data.");
      setIsGeneratingDesc(false);
    }, 1500);
  };

  const getStatusIcon = (status: FileItem["status"]) => {
    switch (status) {
      case "added":
        return <Square className="w-3 h-3 text-green-400" fill="currentColor" />;
      case "modified":
        return <Square className="w-3 h-3 text-yellow-400" fill="currentColor" />;
      case "deleted":
        return <Minus className="w-3 h-3 text-red-400" />;
      case "untracked":
        return <Plus className="w-3 h-3 text-purple-400" />;
    }
  };

  const getFileStatusColor = (status: FileItem["status"]) => {
    switch (status) {
      case "added":
        return "text-green-400";
      case "modified":
        return "text-yellow-400";
      case "deleted":
        return "text-red-400";
      case "untracked":
        return "text-purple-400";
    }
  };

  return (
    <div className="flex flex-col h-screen w-screen overflow-hidden">
      <nav
        className="flex items-center justify-between h-10 px-3 border-b title-bar-drag"
        style={{ backgroundColor: "var(--color-sidebar)", borderColor: "var(--color-border)" }}
      >
        <div className="flex items-center gap-2 title-bar-no-drag">
          <button className="flex items-center gap-2 px-3 py-1 rounded-md hover:bg-white/5 transition-colors">
            <Database className="w-4 h-4" style={{ color: "var(--color-primary)" }} />
            <span className="text-sm font-medium">commitly</span>
            <GitBranch className="w-3 h-3 text-gray-500" />
          </button>
          <button className="flex items-center gap-1.5 px-2 py-1 rounded-md hover:bg-white/5 transition-colors text-sm text-gray-400">
            <GitBranch className="w-3 h-3" />
            <span>main</span>
          </button>
        </div>
        
        <div className="flex items-center gap-1 title-bar-no-drag">
          <span className="text-xs text-gray-500 flex items-center gap-1 pr-2">
            <Clock className="w-3 h-3" />
            {lastFetched}
          </span>
          <button
            className="flex items-center gap-1.5 px-2 py-1 rounded-md hover:bg-white/5 transition-colors text-sm"
            style={{ color: "var(--color-primary)" }}
          >
            <RefreshCcw className="w-3 h-3" />
          </button>
        </div>

        <div className="flex items-center title-bar-no-drag">
          <button 
            onClick={() => { console.log('minimize clicked'); appWindow.minimize(); }} 
            className="title-bar-button"
            title="Minimize"
          >
            <MinusIcon className="w-4 h-4" />
          </button>
          <button 
            onClick={async () => { 
              const maximized = await appWindow.isMaximized();
              if (maximized) {
                await appWindow.unmaximize();
              } else {
                await appWindow.maximize();
              }
            }} 
            className="title-bar-button"
            title="Maximize"
          >
            <SquareIcon className="w-3.5 h-3.5" />
          </button>
          <button 
            onClick={() => { console.log('close clicked'); appWindow.close(); }} 
            className="title-bar-button close"
            title="Close"
          >
            <X className="w-4 h-4" />
          </button>
        </div>
      </nav>

      <div className="flex flex-1 overflow-hidden">
        <aside
          className="flex flex-col w-[300px] border-r transition-sidebar"
          style={{ backgroundColor: "var(--color-sidebar)", borderColor: "var(--color-border)" }}
        >
          <div className="flex border-b" style={{ borderColor: "var(--color-border)" }}>
            <button
              className={`flex-1 px-4 py-2.5 text-sm font-medium transition-tab relative ${
                activeTab === "changes" ? "text-white" : "text-gray-500 hover:text-gray-300"
              }`}
              onClick={() => { setActiveTab("changes"); setSelectedCommit(null); }}
            >
              Changes ({mockFiles.length})
              {activeTab === "changes" && (
                <div
                  className="absolute bottom-0 left-0 right-0 h-0.5"
                  style={{ backgroundColor: "var(--color-primary)" }}
                />
              )}
            </button>
            <button
              className={`flex-1 px-4 py-2.5 text-sm font-medium transition-tab relative ${
                activeTab === "history" ? "text-white" : "text-gray-500 hover:text-gray-300"
              }`}
              onClick={() => setActiveTab("history")}
            >
              History
              {activeTab === "history" && (
                <div
                  className="absolute bottom-0 left-0 right-0 h-0.5"
                  style={{ backgroundColor: "var(--color-primary)" }}
                />
              )}
            </button>
          </div>

          {activeTab === "changes" ? (
            <>
              <div className="flex-1 flex flex-col overflow-hidden">
                <div
                  className="flex items-center gap-2 px-3 py-2 border-b text-sm"
                  style={{ borderColor: "var(--color-border)" }}
                >
                  <button onClick={toggleAll}>
                    {checkedFiles.size === mockFiles.length ? (
                      <CheckSquare className="w-4 h-4" style={{ color: "var(--color-primary)" }} />
                    ) : (
                      <Square className="w-4 h-4 text-gray-500" />
                    )}
                  </button>
                  <span className="text-gray-400">{mockFiles.length} changed files</span>
                </div>
                <div className="flex-1 overflow-y-auto">
                  {mockFiles.map((file) => (
                    <div
                      key={file.name}
                      className={`flex items-center gap-2 px-3 py-1.5 cursor-pointer transition-colors ${
                        selectedFile === file.name ? "bg-white/10" : "hover:bg-white/5"
                      }`}
                      onClick={() => setSelectedFile(file.name)}
                    >
                      <button onClick={(e) => { e.stopPropagation(); toggleFile(file.name); }}>
                        {checkedFiles.has(file.name) ? (
                          <CheckSquare className="w-4 h-4" style={{ color: "var(--color-primary)" }} />
                        ) : (
                          <Square className="w-4 h-4 text-gray-500" />
                        )}
                      </button>
                      <span className="flex-1 text-sm truncate">{file.name}</span>
                      <span className={getFileStatusColor(file.status)}>{getStatusIcon(file.status)}</span>
                    </div>
                  ))}
                </div>
              </div>

              <div
                className="flex flex-col gap-2 p-3 border-t"
                style={{ borderColor: "var(--color-border)" }}
              >
                <div className="flex items-center gap-2">
                  <div
                    className="w-8 h-8 rounded-full flex items-center justify-center"
                    style={{ backgroundColor: "var(--color-primary)" }}
                  >
                    <User className="w-4 h-4 text-white" />
                  </div>
                  <div className="flex-1 relative">
                    <input
                      type="text"
                      placeholder="Summary (required)"
                      value={summary}
                      onChange={(e) => setSummary(e.target.value)}
                      className="w-full px-3 py-1.5 rounded-md text-sm bg-[#010101] border border-[#1a1a1a] focus:border-[#1f6feb] focus:outline-none transition-colors"
                    />
                    <button
                      onClick={generateSummary}
                      className={`absolute right-2 top-1/2 -translate-y-1/2 ${
                        isGeneratingSummary ? "ai-pulse" : ""
                      }`}
                      disabled={isGeneratingSummary}
                    >
                      <Sparkles className="w-3.5 h-3.5 text-gray-500 hover:text-gray-300" />
                    </button>
                  </div>
                </div>
                <div className="relative">
                  <textarea
                    placeholder="Description"
                    value={description}
                    onChange={(e) => setDescription(e.target.value)}
                    rows={2}
                    className="w-full px-3 py-1.5 rounded-md text-sm bg-[#010101] border border-[#1a1a1a] focus:border-[#1f6feb] focus:outline-none transition-colors resize-none"
                  />
                  <button
                    onClick={generateDescription}
                    className={`absolute right-2 bottom-2 ${isGeneratingDesc ? "ai-pulse" : ""}`}
                    disabled={isGeneratingDesc}
                  >
                    <Sparkles className="w-3.5 h-3.5 text-gray-500 hover:text-gray-300" />
                  </button>
                </div>
                <div className="flex items-center gap-2 text-gray-500">
                  <button className="flex items-center gap-1 px-2 py-1 rounded hover:bg-white/5 transition-colors text-xs">
                    <Users className="w-3 h-3" />
                    Add Co-author
                  </button>
                  <button className="flex items-center gap-1 px-2 py-1 rounded hover:bg-white/5 transition-colors text-xs">
                    <Archive className="w-3 h-3" />
                    Options
                  </button>
                </div>
                <button
                  className="w-full py-2 rounded-md font-medium text-sm text-white transition-colors"
                  style={{ backgroundColor: "var(--color-primary)" }}
                  onMouseOver={(e) => (e.currentTarget.style.backgroundColor = "#388bfd")}
                  onMouseOut={(e) => (e.currentTarget.style.backgroundColor = "#1f6feb")}
                >
                  Commit {checkedFiles.size} files to main
                </button>
              </div>
            </>
          ) : (
            <>
              <div className="flex flex-col overflow-hidden">
                <div
                  className="flex items-center gap-2 px-3 py-2 border-b"
                  style={{ borderColor: "var(--color-border)" }}
                >
                  <div className="flex-1 relative">
                    <Search className="absolute left-2 top-1/2 -translate-y-1/2 w-3.5 h-3.5 text-gray-500" />
                    <input
                      type="text"
                      placeholder="Filter commits"
                      value={historySearch}
                      onChange={(e) => setHistorySearch(e.target.value)}
                      className="w-full pl-7 pr-3 py-1.5 rounded-md text-sm bg-[#010101] border border-[#1a1a1a] focus:border-[#1f6feb] focus:outline-none transition-colors"
                    />
                  </div>
                  <button className="p-1.5 rounded hover:bg-white/5 transition-colors ai-pulse">
                    <Sparkles className="w-3.5 h-3.5 text-gray-500 hover:text-gray-300" />
                  </button>
                </div>
                <div className="flex-1 overflow-y-auto">
                  {filteredCommits.map((commit) => (
                    <div
                      key={commit.hash}
                      className={`flex items-start gap-2 px-3 py-2 cursor-pointer transition-colors border-b ${
                        selectedCommit?.hash === commit.hash ? "bg-white/10" : "hover:bg-white/5"
                      }`}
                      style={{ borderColor: "var(--color-border)" }}
                      onClick={() => setSelectedCommit(commit)}
                    >
                      <div className="mt-0.5">
                        <GitCommit className="w-4 h-4 text-gray-500" />
                      </div>
                      <div className="flex-1 min-w-0">
                        <div className="text-sm font-medium truncate">{commit.summary}</div>
                        <div className="text-xs text-gray-500 flex items-center gap-1">
                          <span>{commit.author}</span>
                          <span>·</span>
                          <span>{commit.date}</span>
                        </div>
                      </div>
                      <ChevronRight className="w-3 h-3 text-gray-600 mt-1" />
                    </div>
                  ))}
                </div>
              </div>

              <div
                className="flex items-center justify-between gap-2 p-3 border-t"
                style={{ borderColor: "var(--color-border)", backgroundColor: "#0a0a0a" }}
              >
                <div className="flex items-center gap-2 text-sm text-gray-400">
                  <Download className="w-4 h-4" />
                  <span className="flex items-center gap-1">
                    <Upload className="w-3 h-3" />
                    0 pending
                  </span>
                </div>
                <div className="flex items-center gap-2 text-xs text-gray-500">
                  <span className="flex items-center gap-1">
                    <Upload className="w-3 h-3" />
                    <span>0</span>
                  </span>
                  <span className="flex items-center gap-1">
                    <Download className="w-3 h-3" />
                    <span>0</span>
                  </span>
                </div>
              </div>
            </>
          )}
        </aside>

        <main className="flex-1 flex flex-col overflow-hidden" style={{ backgroundColor: "var(--color-bg)" }}>
          {activeTab === "changes" ? (
            <>
              <div
                className="flex items-center justify-between h-10 px-4 border-b"
                style={{ borderColor: "var(--color-border)" }}
              >
                <div className="flex items-center gap-2">
                  <FileText className="w-4 h-4 text-gray-500" />
                  <span className="text-sm font-mono">{selectedFile}</span>
                </div>
                <div className="flex items-center gap-1">
                  <button className="p-1.5 rounded hover:bg-white/5 transition-colors">
                    <Settings className="w-4 h-4 text-gray-500" />
                  </button>
                  <button className="p-1.5 rounded hover:bg-white/5 transition-colors">
                    <Maximize2 className="w-4 h-4 text-gray-500" />
                  </button>
                </div>
              </div>

              <div className="flex-1 overflow-auto font-mono text-sm">
                {currentDiff.map((line, idx) => {
                  if (line.type === "header") {
                    return (
                      <div
                        key={idx}
                        className="px-4 py-1 text-gray-500"
                        style={{ backgroundColor: "#121212" }}
                      >
                        {line.content}
                      </div>
                    );
                  }
                  return (
                    <div
                      key={idx}
                      className={`flex ${
                        line.type === "added"
                          ? "bg-[#1f3d1f]"
                          : line.type === "removed"
                          ? "bg-[#3d1f1f]"
                          : ""
                      }`}
                    >
                      <span className="w-12 px-3 py-0.5 text-right text-gray-600 select-none border-r border-[#1a1a1a]">
                        {line.num || ""}
                      </span>
                      <span className={`w-6 px-2 py-0.5 select-none ${
                        line.type === "added" ? "text-green-400" : line.type === "removed" ? "text-red-400" : ""
                      }`}>
                        {line.type === "added" ? "+" : line.type === "removed" ? "-" : ""}
                      </span>
                      <span className="flex-1 px-2 py-0.5 whitespace-pre">
                        {line.content}
                      </span>
                    </div>
                  );
                })}
              </div>
            </>
          ) : (
            <>
              <div
                className="flex items-center justify-between h-10 px-4 border-b"
                style={{ borderColor: "var(--color-border)" }}
              >
                <div className="flex items-center gap-2">
                  <GitCommit className="w-4 h-4 text-gray-500" />
                  {selectedCommit ? (
                    <span className="text-sm font-mono">{selectedCommit.hash}</span>
                  ) : (
                    <span className="text-sm text-gray-500">Select a commit to view details</span>
                  )}
                </div>
                <div className="flex items-center gap-1">
                  <button className="p-1.5 rounded hover:bg-white/5 transition-colors">
                    <Settings className="w-4 h-4 text-gray-500" />
                  </button>
                </div>
              </div>

              {selectedCommit ? (
                <div className="flex-1 overflow-auto">
                  <div className="p-4 border-b" style={{ borderColor: "var(--color-border)" }}>
                    <div className="text-lg font-medium">{selectedCommit.summary}</div>
                    <div className="flex items-center gap-2 mt-1 text-sm text-gray-500">
                      <User className="w-3.5 h-3.5" />
                      <span>{selectedCommit.author}</span>
                      <span>·</span>
                      <span>{selectedCommit.date}</span>
                    </div>
                    <div className="mt-2 text-sm text-gray-400">
                      Updated {selectedCommit.files.length} files
                    </div>
                  </div>

                  <div className="px-4 py-2 border-b" style={{ borderColor: "var(--color-border)", backgroundColor: "#121212" }}>
                    <div className="text-xs font-medium text-gray-500 mb-1">FILES CHANGED</div>
                    {selectedCommit.files.map((file, idx) => (
                      <div key={idx} className="text-sm py-0.5">{file}</div>
                    ))}
                  </div>

                  <div className="font-mono text-sm">
                    {historyCommitDiff.map((line, idx) => {
                      if (line.type === "header") {
                        return (
                          <div
                            key={idx}
                            className="px-4 py-1 text-gray-500"
                            style={{ backgroundColor: "#0a0a0a" }}
                          >
                            {line.content}
                          </div>
                        );
                      }
                      return (
                        <div
                          key={idx}
                          className={`flex ${
                            line.type === "added"
                              ? "bg-[#1f3d1f]"
                              : line.type === "removed"
                              ? "bg-[#3d1f1f]"
                              : ""
                          }`}
                        >
                          <span className="w-12 px-3 py-0.5 text-right text-gray-600 select-none border-r border-[#1a1a1a]">
                            {line.num || ""}
                          </span>
                          <span className={`w-6 px-2 py-0.5 select-none ${
                            line.type === "added" ? "text-green-400" : line.type === "removed" ? "text-red-400" : ""
                          }`}>
                            {line.type === "added" ? "+" : line.type === "removed" ? "-" : ""}
                          </span>
                          <span className="flex-1 px-2 py-0.5 whitespace-pre">
                            {line.content}
                          </span>
                        </div>
                      );
                    })}
                  </div>
                </div>
              ) : (
                <div className="flex-1 flex items-center justify-center text-gray-500">
                  <div className="text-center">
                    <GitCommit className="w-12 h-12 mx-auto mb-2 opacity-50" />
                    <div>Select a commit from the history to view details</div>
                  </div>
                </div>
              )}
            </>
          )}
        </main>
      </div>
    </div>
  );
}

export default App;