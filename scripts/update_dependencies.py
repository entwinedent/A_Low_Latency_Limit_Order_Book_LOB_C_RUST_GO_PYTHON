#!/usr/bin/env python3
"""
Dependency Update Script for Low-Latency Order Book Engine

This script helps update and manage dependencies across different package managers.
"""

import argparse
import subprocess
import sys
from pathlib import Path


def run_command(cmd, description):
    """Run a command and handle errors."""
    print(f"\n{description}...")
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        print(f"✓ {description} completed successfully")
        return True
    except subprocess.CalledProcessError as e:
        print(f"✗ {description} failed: {e}")
        print(f"Error output: {e.stderr}")
        return False


def update_conan():
    """Update Conan dependencies."""
    print("\n=== Updating Conan Dependencies ===")
    
    commands = [
        ("conan install . --output-folder=build --build=missing", 
         "Installing Conan dependencies"),
        ("conan remove * -- outdated", 
         "Removing outdated Conan packages"),
    ]
    
    success = True
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            success = False
    
    return success


def update_vcpkg():
    """Update vcpkg dependencies."""
    print("\n=== Updating vcpkg Dependencies ===")
    
    commands = [
        ("vcpkg update", 
         "Updating vcpkg package database"),
        ("vcpkg upgrade", 
         "Upgrading vcpkg packages"),
    ]
    
    success = True
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            success = False
    
    return success


def update_go():
    """Update Go dependencies."""
    print("\n=== Updating Go Dependencies ===")
    
    go_mod_path = Path("bindings/go")
    if not go_mod_path.exists():
        print("✗ Go bindings directory not found")
        return False
    
    commands = [
        (f"cd {go_mod_path} && go mod download", 
         "Downloading Go modules"),
        (f"cd {go_mod_path} && go mod tidy", 
         "Tidying Go modules"),
        (f"cd {go_mod_path} && go get -u ./...", 
         "Updating Go dependencies"),
    ]
    
    success = True
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            success = False
    
    return success


def update_rust():
    """Update Rust dependencies."""
    print("\n=== Updating Rust Dependencies ===")
    
    rust_path = Path("bindings/rust")
    if not rust_path.exists():
        print("✗ Rust bindings directory not found")
        return False
    
    commands = [
        (f"cd {rust_path} && cargo update", 
         "Updating Cargo dependencies"),
        (f"cd {rust_path} && cargo upgrade", 
         "Upgrading Cargo dependencies (if cargo-upgrade installed)"),
    ]
    
    success = True
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            # cargo-upgrade might not be installed, don't fail
            if "cargo-upgrade" in desc:
                print("  (Optional: install cargo-edit for dependency upgrades)")
            else:
                success = False
    
    return success


def update_python():
    """Update Python dependencies."""
    print("\n=== Updating Python Dependencies ===")
    
    python_path = Path("bindings/python")
    if not python_path.exists():
        print("✗ Python bindings directory not found")
        return False
    
    commands = [
        ("pip install --upgrade pip", 
         "Upgrading pip"),
        (f"cd {python_path} && pip install --upgrade -r requirements.txt", 
         "Upgrading Python dependencies"),
    ]
    
    success = True
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            success = False
    
    return success


def update_all():
    """Update all dependencies."""
    print("=== Updating All Dependencies ===")
    
    results = {
        "Conan": update_conan(),
        "vcpkg": update_vcpkg(),
        "Go": update_go(),
        "Rust": update_rust(),
        "Python": update_python(),
    }
    
    print("\n=== Update Summary ===")
    for component, success in results.items():
        status = "✓ Success" if success else "✗ Failed"
        print(f"{component}: {status}")
    
    return all(results.values())


def main():
    parser = argparse.ArgumentParser(
        description="Update dependencies for Low-Latency Order Book Engine"
    )
    parser.add_argument(
        "component",
        nargs="?",
        choices=["all", "conan", "vcpkg", "go", "rust", "python"],
        default="all",
        help="Component to update (default: all)"
    )
    
    args = parser.parse_args()
    
    if args.component == "all":
        success = update_all()
    elif args.component == "conan":
        success = update_conan()
    elif args.component == "vcpkg":
        success = update_vcpkg()
    elif args.component == "go":
        success = update_go()
    elif args.component == "rust":
        success = update_rust()
    elif args.component == "python":
        success = update_python()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()