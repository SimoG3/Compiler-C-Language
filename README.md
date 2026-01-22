# Compiler---C-Language
This project involves the creation of a complete compiler for a specific algorithmic language. Developed in C, this software is capable of transforming textual source code into an organized data structure, verifying its grammatical validity, and ensuring the logical consistency of operations.

# 🛠️ C-Algorithmic Compiler

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Build](https://img.shields.io/badge/Build-GCC-orange.svg)
![Status](https://img.shields.io/badge/Status-Complete-green.svg)

## 📝 Project Description

This project involves the creation of a complete compiler for a specific algorithmic language. Developed in **C**, this software is capable of transforming textual source code into an organized data structure, verifying its grammatical validity, and ensuring the logical consistency of operations (notably type management).

More than just a translator, this project is a robust diagnostic tool that ensures an algorithmic program is perfectly compliant before any execution. It serves as a foundation for understanding the mechanics of high-level programming languages.



---

## 🏗️ Technical Architecture

The compiler is built with a modular architecture, simulating the standard phases of industrial compilers:

### 1. Lexical Analysis (The Lexer)
The program reads the source file and breaks it down into elementary units called **tokens** (keywords, identifiers, symbols). 
* Uses a **dynamic linked list** for storage.
* Validates words against a reference dictionary (`dictionnaire.txt`).
* Filters out comments and handles string literals.

### 2. Syntax Analysis (The Parser)
This stage verifies if the arrangement of tokens respects the formal grammar of the language.
* Checks for mandatory structures (e.g., `debut:` and `Fin.`).
* Validates instruction endings (`;`) and balanced parentheses.
* Uses a state-machine logic with labels (`goto`) to handle nested instructions.

### 3. Symbol Table Management
During analysis, the compiler dynamically registers every declared variable, its name, and its type (`Entier` or `Reel`) into a dedicated symbol table for contextual validation.



### 4. Semantic Analysis
The "intelligence" of the compiler. It ensures the code makes sense logically:
* **Type Checking:** Prevents assigning a `Reel` value to an `Entier` variable to avoid data loss.
* **Declaration Check:** Ensures variables are declared before being used in assignments or I/O operations.

---

## ✨ Key Features

* **I/O Operations:** Support for `Lire()` and `Ecrire()` with string literal support.
* **Control Structures:** Implementation of conditional blocks: `Si ... Alors ... Sinon ... FinSi`.
* **Strong Typing:** Strict distinction between integer and real types.
* **Error Reporting:** Precise diagnostic messages including **Error Type** (Lexical, Syntax, or Semantic) and **Line Number**, logged to both console and `errors.txt`.

---

## 🚀 Getting Started

### Installation
1. Clone the repository:
   ```pascal
   git clone [https://github.com/SimoG3/Compiler-C-Language.git]
   ```
2. Navigate to the folder and compile:
   ```pascal
   gcc -o compiler main.c
   ```
Running the Compiler
   Make sure your source code is in programme.txt and your keywords are in dictionnaire.txt, then run:
   ```pascal
   ./compiler
   ```

### 📄 Example Algorithmic Code
   ```pascal
   debut:
  var A : Entier;
  var B : Reel;
  
  Ecrire("Entrez une valeur :");
  Lire(A);
  
  B = 5.5;
  
  Si (A > 10) Alors
    Ecrire("A est grand");
  Sinon
    Ecrire("A est petit");
  FinSi
Fin.
  ```

### 👤 Author
   **Mohamed Benabdallah Belbaraka** *ENSA Berrechid - Academic Year 2025/2026 Supervised by: Mr. Hnini*
