# ⚙️ Kernel Query Execution & Paging System

<p align="center">
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=23&duration=3000&pause=1000&color=00D9FF&center=true&vCenter=true&width=800&lines=Operating+Systems+%2B+DBMS;Kernel-Level+Algorithm+Simulation;Demand+Paging+%7C+CPU+Scheduling;Oracle-Powered+Execution+Persistence" alt="Typing SVG" />
</p>

<p align="center">
  <strong>A unified simulation environment for Operating System algorithms and database-driven execution.</strong>
</p>

<p align="center">
  <a href="https://github.com/shashwat123-creator/kernel-query-execution-paging-system">
    <img src="https://img.shields.io/badge/Project-OS%20%2B%20DBMS-007ACC?style=for-the-badge&logo=linux&logoColor=white" />
  </a>
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-blue?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Database-Oracle-red?style=for-the-badge&logo=oracle&logoColor=white" />
  <img src="https://img.shields.io/badge/Frontend-HTML%20%7C%20CSS%20%7C%20JS-orange?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Status-Active-success?style=for-the-badge" />
</p>

<p align="center">
  <a href="#-overview">Overview</a> •
  <a href="#-modules">Modules</a> •
  <a href="#-architecture">Architecture</a> •
  <a href="#-api-reference">API</a> •
  <a href="#-setup">Setup</a>
</p>

---

## 🖥️ Project at a Glance

| Component | Implementation |
|---|---|
| Core Engine | C++17 |
| Database | Oracle Database Free |
| Database Connectivity | Oracle OCCI |
| Backend | C++ REST API |
| Frontend | HTML, CSS, JavaScript |
| Deployment | Docker |
| Memory Algorithms | FIFO, LRU, Optimal |
| Scheduling | FCFS, Round Robin |

<p align="center">
  <i>Simulate. Analyze. Persist. Understand.</i>
</p>

---

## ✨ Overview

Kernel Query Execution & Paging System integrates fundamental Operating System algorithms with relational database management through a unified web dashboard.

The platform allows users to execute algorithms, visualize intermediate states, compare execution results, and persist simulation records in Oracle Database.

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=cplusplus" />
  <img src="https://img.shields.io/badge/Oracle-Database-F80000?style=flat-square&logo=oracle" />
  <img src="https://img.shields.io/badge/JavaScript-ES6-F7DF1E?style=flat-square&logo=javascript" />
  <img src="https://img.shields.io/badge/HTML5-E34F26?style=flat-square&logo=html5" />
  <img src="https://img.shields.io/badge/CSS3-1572B6?style=flat-square&logo=css3" />
  <img src="https://img.shields.io/badge/Docker-2496ED?style=flat-square&logo=docker" />
</p>
## 🏗️ System Architecture

```mermaid
flowchart TD
    A["User"] --> B["Web Dashboard"]
    B --> C["C++ Backend API"]

    C --> D["CPU Scheduling"]
    C --> E["Deadlock Avoidance"]
    C --> F["File Allocation"]
    C --> G["Demand Paging"]

    H["Process Monitor Agent"] --> I["Oracle Database"]

    D --> I
    E --> I
    F --> I
    G --> I

    I --> C
    C --> B

    style A fill:#2563eb,color:#fff
    style B fill:#0f766e,color:#fff
    style C fill:#7c3aed,color:#fff
    style I fill:#b91c1c,color:#fff
```

## 🧠 Algorithm Explorer

<details>
<summary><strong>⚡ CPU Scheduling</strong></summary>

- First Come First Serve (FCFS)
- Round Robin (RR)
- Execution order and scheduling metrics

</details>

<details>
<summary><strong>🔒 Deadlock Avoidance</strong></summary>

- Banker's Algorithm
- Allocation, Maximum and Need matrices
- Safe-state detection
- Safe sequence generation

</details>

<details>
<summary><strong>📁 Contiguous File Allocation</strong></summary>

- Consecutive disk block allocation
- File storage representation
- Allocation visualization

</details>

<details>
<summary><strong>🧮 Demand Paging</strong></summary>

- FIFO Page Replacement
- LRU Page Replacement
- Optimal Page Replacement
- Page hits and page faults
- Frame-by-frame execution visualization
- Algorithm comparison and Oracle persistence

</details>

<details>
<summary><strong>📊 Process Monitoring</strong></summary>

- Process identification
- CPU utilization
- Memory utilization
- Periodic resource measurements

</details>

| Algorithm | Page Hits | Page Faults | Fault Ratio |
|---|---:|---:|---:|
| FIFO | 3 | 10 | 76.92% |
| LRU | 4 | 9 | 69.23% |
| Optimal | 6 | 7 | 53.85% |
## 📈 Repository Insights

<p align="center">
  <img
    src="https://github-readme-stats.vercel.app/api/pin/?username=shashwat123-creator&repo=kernel-query-execution-paging-system&theme=tokyonight"
    width="400"
  />
</p>

<p align="center">
  <img
    src="https://img.shields.io/github/last-commit/shashwat123-creator/kernel-query-execution-paging-system?style=for-the-badge"
  />
  <img
    src="https://img.shields.io/github/languages/top/shashwat123-creator/kernel-query-execution-paging-system?style=for-the-badge"
  />
  <img
    src="https://img.shields.io/github/repo-size/shashwat123-creator/kernel-query-execution-paging-system?style=for-the-badge"
  />
</p>
