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
} from "lucide-react";
import { useHotkey } from "@tanstack/react-hotkeys";
import "./App.css";

interface FileItem {
  name: string;
  status: "added" | "modified" | "deleted" | "untracked";
}

const mockFiles: FileItem[] = [
  { name: ".gitignore", status: "modified" },
  { name: "src/App.tsx", status: "modified" },
  { name: "src/main.tsx", status: "modified" },
  { name: "package.json", status: "modified" },
  { name: "vite.config.ts", status: "modified" },
  { name: "tsconfig.json", status: "modified" },
  { name: "README.md", status: "added" },
  { name: "src/components/Header.tsx", status: "added" },
  { name: "src/components/Sidebar.tsx", status: "added" },
  { name: "src/utils/api.ts", status: "untracked" },
  { name: "src/styles/global.css", status: "modified" },
  { name: ".env.local", status: "untracked" },
  { name: "src/hooks/useAuth.ts", status: "added" },
  { name: "src/lib/db.ts", status: "modified" },
  { name: "src/types/index.ts", status: "modified" },
  { name: "public/logo.svg", status: "added" },
  { name: "src/pages/Dashboard.tsx", status: "modified" },
  { name: "src/pages/Settings.tsx", status: "modified" },
  { name: "src/services/auth.ts", status: "deleted" },
  { name: "src/utils/helpers.ts", status: "modified" },
];

type DiffLine = { type: "header"; content: string } | { type: "added" | "removed"; num?: number; content: string };

const mockDiff: DiffLine[] = [
  { type: "header", content: "@@ -0,0 +1,24 @@" },
  { type: "added", num: 1, content: "# Commitly" },
  { type: "added", num: 2, content: "" },
  { type: "added", num: 3, content: "A powerful Git desktop client for modern developers." },
  { type: "added", num: 4, content: "" },
  { type: "added", num: 5, content: "## Features" },
  { type: "added", num: 6, content: "" },
  { type: "added", num: 7, content: "- AI-powered commit messages" },
  { type: "added", num: 8, content: "- Real-time collaboration" },
  { type: "added", num: 9, content: "- Beautiful dark mode UI" },
  { type: "added", num: 10, content: "- Branch management" },
  { type: "added", num: 11, content: "" },
  { type: "added", num: 12, content: "## Installation" },
  { type: "added", num: 13, content: "" },
  { type: "added", num: 14, content: "npm install commitly" },
  { type: "added", num: 15, content: "## Usage" },
  { type: "added", num: 16, content: "" },
  { type: "added", num: 17, content: "Run commitly init to get started." },
  { type: "added", num: 18, content: "" },
  { type: "added", num: 19, content: "## License" },
  { type: "added", num: 20, content: "" },
  { type: "added", num: 21, content: "MIT" },
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
    { type: "added", num: 7, content: "" },
    { type: "added", num: 8, content: "function App() {" },
    { type: "added", num: 9, content: "  const [count, setCount] = useState(0);" },
    { type: "added", num: 10, content: "" },
    { type: "added", num: 11, content: "  return (" },
    { type: "added", num: 12, content: "    <div>" },
  ],
  "src/main.tsx": [
    { type: "header", content: "@@ -3,10 +3,12 @@" },
    { type: "added", num: 3, content: "import App from './App';" },
    { type: "added", num: 4, content: "import './index.css';" },
    { type: "added", num: 5, content: "" },
    { type: "added", num: 6, content: "createRoot(document.getElementById('root')!).render(" },
    { type: "added", num: 7, content: "  <StrictMode>" },
    { type: "added", num: 8, content: "    <App />" },
    { type: "added", num: 9, content: "  </StrictMode>," },
    { type: "added", num: 10, content: ");" },
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
    { type: "added", num: 8, content: '    "preview": "vite preview",' },
    { type: "added", num: 9, content: '    "tauri": "tauri"' },
    { type: "added", num: 10, content: "  }," },
    { type: "added", num: 11, content: '  "dependencies": {' },
    { type: "added", num: 12, content: '    "react": "^19.1.0",' },
  ],
  "README.md": [
    { type: "header", content: "@@ -0,0 +1,15 @@" },
    { type: "added", num: 1, content: "# Commitly" },
    { type: "added", num: 2, content: "" },
    { type: "added", num: 3, content: "A modern Git desktop client built with Tauri and React." },
    { type: "added", num: 4, content: "" },
    { type: "added", num: 5, content: "## Getting Started" },
    { type: "added", num: 6, content: "" },
    { type: "added", num: 7, content: "npm install" },
    { type: "added", num: 8, content: "npm run tauri dev" },
    { type: "added", num: 9, content: "" },
    { type: "added", num: 10, content: "## Features" },
    { type: "added", num: 11, content: "" },
    { type: "added", num: 12, content: "- AI-powered commit messages" },
    { type: "added", num: 13, content: "- Beautiful dark mode UI" },
    { type: "added", num: 14, content: "- Real-time diff viewing" },
    { type: "added", num: 15, content: "- Branch management" },
  ],
  "src/components/Header.tsx": [
    { type: "header", content: "@@ -0,0 +1,22 @@" },
    { type: "added", num: 1, content: "interface HeaderProps {" },
    { type: "added", num: 2, content: "  title: string;" },
    { type: "added", num: 3, content: "  onMenuClick?: () => void;" },
    { type: "added", num: 4, content: "}" },
    { type: "added", num: 5, content: "" },
    { type: "added", num: 6, content: "export function Header({ title, onMenuClick }: HeaderProps) {" },
    { type: "added", num: 7, content: "  return (" },
    { type: "added", num: 8, content: '    <header className="flex items-center justify-between">' },
    { type: "added", num: 9, content: "      <h1>{title}</h1>" },
    { type: "added", num: 10, content: "      <button onClick={onMenuClick}>Menu</button>" },
    { type: "added", num: 11, content: "    </header>" },
    { type: "added", num: 12, content: "  );" },
    { type: "added", num: 13, content: "}" },
  ],
  ".env.local": [
    { type: "header", content: "@@ -0,0 +1,5 @@" },
    { type: "added", num: 1, content: "VITE_API_URL=http://localhost:3000" },
    { type: "added", num: 2, content: "VITE_WS_URL=ws://localhost:3001" },
    { type: "added", num: 3, content: "TAURI_DEBUG=true" },
    { type: "added", num: 4, content: "" },
    { type: "added", num: 5, content: "# Add your API keys below" },
  ],
  "src/services/auth.ts": [
    { type: "header", content: "@@ -1,15 +0,0 @@" },
    { type: "removed", num: 1, content: "export function login(email: string) {" },
    { type: "removed", num: 2, content: "  return fetch('/api/auth/login', {" },
    { type: "removed", num: 3, content: "    method: 'POST'," },
    { type: "removed", num: 4, content: "    body: JSON.stringify({ email })," },
    { type: "removed", num: 5, content: "  });" },
    { type: "removed", num: 6, content: "}" },
    { type: "removed", num: 7, content: "" },
    { type: "removed", num: 8, content: "export function logout() {" },
    { type: "removed", num: 9, content: "  localStorage.removeItem('token');" },
    { type: "removed", num: 10, content: "  window.location.reload();" },
    { type: "removed", num: 11, content: "}" },
  ],
  "vite.config.ts": [
    { type: "header", content: "@@ -1,8 +1,12 @@" },
    { type: "added", num: 1, content: "import { defineConfig } from 'vite';" },
    { type: "added", num: 2, content: "import react from '@vitejs/plugin-react';" },
    { type: "added", num: 3, content: "import tailwindcss from '@tailwindcss/vite';" },
    { type: "added", num: 4, content: "" },
    { type: "added", num: 5, content: "export default defineConfig(async () => ({" },
    { type: "added", num: 6, content: "  plugins: [react(), tailwindcss()]," },
    { type: "added", num: 7, content: "  server: {" },
    { type: "added", num: 8, content: "    port: 1420," },
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

  const navigateToFile = useCallback((index: number) => {
    const clampedIndex = Math.max(0, Math.min(index, mockFiles.length - 1));
    setSelectedIndex(clampedIndex);
    setSelectedFile(mockFiles[clampedIndex].name);
  }, []);

  useHotkey('ArrowDown', () => {
    if (activeTab === "changes") {
      navigateToFile(selectedIndex + 1);
    }
  });

  useHotkey('ArrowUp', () => {
    if (activeTab === "changes") {
      navigateToFile(selectedIndex - 1);
    }
  });

  useHotkey({ key: 'ArrowRight', ctrl: true }, () => {
    setActiveTab("history");
  });

  useHotkey({ key: 'ArrowLeft', ctrl: true }, () => {
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
      setDescription("Added node_modules and environment files to avoid committing sensitive data and dependencies.");
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
        className="flex items-center justify-between h-12 px-4 border-b"
        style={{ backgroundColor: "var(--color-sidebar)", borderColor: "var(--color-border)" }}
        data-tauri-drag-region
      >
        <div className="flex items-center gap-3">
          <button className="flex items-center gap-2 px-3 py-1.5 rounded-md hover:bg-white/5 transition-colors">
            <Database className="w-4 h-4" style={{ color: "var(--color-primary)" }} />
            <span className="text-sm font-medium">commitly</span>
            <GitBranch className="w-3 h-3 text-gray-500" />
          </button>
          <button className="flex items-center gap-1.5 px-2 py-1.5 rounded-md hover:bg-white/5 transition-colors text-sm text-gray-400">
            <GitBranch className="w-3 h-3" />
            <span>main</span>
          </button>
        </div>
        <div className="flex items-center gap-2">
          <span className="text-xs text-gray-500 flex items-center gap-1">
            <Clock className="w-3 h-3" />
            Last fetched: {lastFetched}
          </span>
          <button
            className="flex items-center gap-1.5 px-3 py-1.5 rounded-md hover:bg-white/5 transition-colors text-sm"
            style={{ color: "var(--color-primary)" }}
          >
            <RefreshCcw className="w-3 h-3" />
            Fetch origin
          </button>
        </div>
      </nav>

      <div className="flex flex-1 overflow-hidden">
        <aside
          className="flex flex-col w-[300px] border-r"
          style={{ backgroundColor: "var(--color-sidebar)", borderColor: "var(--color-border)" }}
        >
          <div className="flex border-b" style={{ borderColor: "var(--color-border)" }}>
            <button
              className={`flex-1 px-4 py-2.5 text-sm font-medium transition-colors relative ${
                activeTab === "changes" ? "text-white" : "text-gray-500 hover:text-gray-300"
              }`}
              onClick={() => setActiveTab("changes")}
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
              className={`flex-1 px-4 py-2.5 text-sm font-medium transition-colors relative ${
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
                  className="w-full px-3 py-1.5 rounded-md text-sm bg-[#0d1117] border border-[#30363d] focus:border-[#1f6feb] focus:outline-none transition-colors"
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
                className="w-full px-3 py-1.5 rounded-md text-sm bg-[#0d1117] border border-[#30363d] focus:border-[#1f6feb] focus:outline-none transition-colors resize-none"
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
        </aside>

        <main className="flex-1 flex flex-col overflow-hidden" style={{ backgroundColor: "var(--color-bg)" }}>
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
                    style={{ backgroundColor: "#161b22" }}
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
                  <span className="w-12 px-3 py-0.5 text-right text-gray-600 select-none border-r border-[#30363d]">
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
        </main>
      </div>
    </div>
  );
}

export default App;