# Support

This document provides information on how to get support for the Low Latency Limit Order Book (LOB) project.

## Getting Help

### Documentation
- **README.md** - Project overview, setup instructions, and quick start guide
- **ARCHITECTURE.md** - Detailed architecture documentation and design decisions
- **SETUP.md** - Step-by-step setup instructions for different platforms
- **CHANGELOG.md** - Version history and release notes
- **ROADMAP.md** - Project roadmap and planned features

### Community Support
- **GitHub Issues** - Report bugs and request features
- **GitHub Discussions** - Ask questions and share ideas
- **GitHub Wiki** - Community-contributed documentation and tutorials

### Professional Support
For enterprise support, custom development, or consulting services, please contact the maintainers directly.

## Common Issues

### Build Issues

**CMake Version Errors**
- Ensure you have CMake 3.28 or higher installed
- Check that your compiler supports C++23 (GCC 13+, MSVC 19.35+, Clang 15+)

**C++23 Feature Compilation Errors**
- Some C++23 features require specific compiler versions
- Check `cmake/CheckCXX23.cmake` for feature detection results
- Use Docker environment for consistent C++23 testing (GCC 16.0.1)

**Docker Build Failures**
- Ensure Docker is running and has sufficient disk space
- Check that Docker Buildx is properly configured
- Try clearing Docker cache: `docker system prune -a --volumes -f`

### Runtime Issues

**Performance Issues**
- Ensure you're building in Release mode
- Check that compiler optimizations are enabled
- Review benchmark results for baseline performance
- Consider using the Docker environment for consistent performance testing

**Memory Issues**
- The memory pool has a fixed capacity (configurable in CMake)
- Monitor memory usage with system tools
- Adjust memory pool size for your workload

**Language Binding Issues**
- Ensure the C++ library is built before testing bindings
- Check that language toolchains are properly installed
- Review binding-specific documentation in `bindings/` directories

## Reporting Bugs

When reporting bugs, please include:

1. **Environment Information**
   - OS and version
   - Compiler and version
   - CMake version
   - Build type (Debug/Release)

2. **Minimal Reproduction**
   - Code sample that reproduces the issue
   - Build log with errors
   - Expected vs actual behavior

3. **C++23 Features**
   - Which C++23 features you're using
   - Whether you're using Docker environment

Use the bug report template in `.github/ISSUE_TEMPLATE/bug_report.md`.

## Feature Requests

For feature requests, please:

1. Check the `ROADMAP.md` to see if the feature is already planned
2. Search existing issues to avoid duplicates
3. Provide motivation and use cases
4. Consider implementation complexity and performance impact

Use the feature request template in `.github/ISSUE_TEMPLATE/feature_request.md`.

## Contributing

We welcome contributions! Please see:

- **CONTRIBUTING.md** - Contribution guidelines
- **CODE_OF_CONDUCT.md** - Community code of conduct
- `.github/pull_request_template.md` - PR template

## Security Issues

For security vulnerabilities, please:

1. Do not open a public issue
2. Contact maintainers directly via private channels
3. Provide detailed information about the vulnerability
4. Allow time for patching before disclosure

See `SECURITY.md` for more details.

## Performance Support

For performance-related issues:

1. Include benchmark results
2. Provide profiling data if available
3. Describe your workload and requirements
4. Specify your hardware environment

Use the performance issue template in `.github/ISSUE_TEMPLATE/performance_issue.md`.

## Language Binding Support

### Go Bindings
- Documentation: `bindings/go/README.md`
- Issues: Tag with `go-binding` label

### Rust Bindings
- Documentation: `bindings/rust/README.md`
- Issues: Tag with `rust-binding` label

### Python Bindings
- Documentation: `bindings/python/README.md`
- Issues: Tag with `python-binding` label

## Professional Services

For enterprise support, custom development, or training:

- Contact: [Your Email]
- Website: [Your Website]
- Consulting: Available upon request

## Response Times

- **Bug Reports**: 2-3 business days
- **Feature Requests**: 1-2 weeks (for initial review)
- **Security Issues**: Within 24 hours
- **Enterprise Support**: SLA-based

## Additional Resources

- **C++23 Reference**: https://en.cppreference.com/w/cpp/23
- **Kokkos mdspan**: https://github.com/kokkos/mdspan
- **GoogleTest**: https://google.github.io/googletest/
- **spdlog**: https://github.com/gabime/spdlog

---

**Last Updated**: August 2026
