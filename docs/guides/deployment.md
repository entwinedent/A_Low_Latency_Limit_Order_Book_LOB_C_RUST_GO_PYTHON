# Deployment Guide

This guide covers deployment strategies and best practices for the Low-Latency Order Book Engine in production environments.

## Deployment Models

### Standalone Deployment
- **Description**: Single binary or library running on dedicated hardware
- **Use Case**: High-frequency trading desks, maximum performance requirements
- **Advantages**: Minimal latency, maximum control, simplified deployment
- **Disadvantages**: Limited scalability, single point of failure

### Service Deployment
- **Description**: Network-accessible service with API endpoints
- **Use Case**: Multi-client trading platforms, shared infrastructure
- **Advantages**: Centralized management, resource sharing, easier scaling
- **Disadvantages**: Network latency, increased complexity

### Container Deployment
- **Description**: Docker container with all dependencies
- **Use Case**: Cloud deployment, testing environments, microservices
- **Advantages**: Consistency, scalability, isolation
- **Disadvantages**: Performance overhead, container management

### Distributed Deployment
- **Description**: Multiple instances working together
- **Use Case**: Large-scale platforms, high availability requirements
- **Advantages**: Scalability, redundancy, geographic distribution
- **Disadvantages**: Complexity, consistency challenges

## Pre-Deployment Checklist

### System Requirements
- [ ] CPU: Modern processor with AVX2 support
- [ ] Memory: Minimum 8GB RAM, 16GB+ recommended
- [ ] Storage: SSD for low-latency I/O
- [ ] Network: Low-latency network interface (if service deployment)
- [ ] OS: Linux kernel 5.4+, Windows 10+, macOS 11+

### Software Dependencies
- [ ] CMake 3.20+
- [ ] C++20 compatible compiler
- [ ] Go 1.21+ (if using Go bindings)
- [ ] Rust 1.70+ (if using Rust bindings)
- [ ] Python 3.8+ (if using Python bindings)

### Configuration
- [ ] Environment variables configured
- [ ] Risk management limits set
- [ ] Memory pool size configured
- [ ] Logging level set appropriately
- [ ] Monitoring enabled

### Security
- [ ] Firewall rules configured
- [ ] Authentication set up (if service deployment)
- [ ] TLS certificates (if using network communication)
- [ ] Security policies reviewed
- [ ] Access controls implemented

## Standalone Deployment

### Linux Deployment
```bash
# Build for production
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Install
sudo cmake --install build --prefix /opt/lob-engine

# Configure
sudo cp .env /etc/lob-engine/env
sudoedit /etc/lob-engine/env

# Run
/opt/lob-engine/bin/OrderBookCLI
```

### Windows Deployment
```bash
# Build for production
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Install
cmake --install build --prefix C:\lob-engine

# Configure
copy .env C:\lob-engine\config\env
notepad C:\lob-engine\config\env

# Run
C:\lob-engine\bin\OrderBookCLI.exe
```

### macOS Deployment
```bash
# Build for production
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Install
sudo cmake --install build --prefix /usr/local/lob-engine

# Configure
sudo cp .env /usr/local/etc/lob-engine/env
sudo nano /usr/local/etc/lob-engine/env

# Run
/usr/local/lob-engine/bin/OrderBookCLI
```

## Service Deployment

### Systemd Service (Linux)
```ini
# /etc/systemd/system/lob-engine.service
[Unit]
Description=Low-Latency Order Book Engine
After=network.target

[Service]
Type=simple
User=lob-engine
Group=lob-engine
WorkingDirectory=/opt/lob-engine
EnvironmentFile=/etc/lob-engine/env
ExecStart=/opt/lob-engine/bin/OrderBookService
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

```bash
# Install service
sudo cp lob-engine.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable lob-engine
sudo systemctl start lob-engine

# Check status
sudo systemctl status lob-engine
```

### Windows Service
```bash
# Use NSSM (Non-Sucking Service Manager)
nssm install OrderBookEngine C:\lob-engine\bin\OrderBookService.exe
nssm set OrderBookEngine AppDirectory C:\lob-engine\bin
nssm set OrderBookEngine AppEnvironmentExtra "MEMORY_POOL_SIZE=1000000"
nssm start OrderBookEngine
```

### Launchd Service (macOS)
```xml
# ~/Library/LaunchAgents/com.lob-engine.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.lob-engine</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/lob-engine/bin/OrderBookService</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

```bash
# Load service
launchctl load ~/Library/LaunchAgents/com.lob-engine.plist
launchctl start com.lob-engine
```

## Container Deployment

### Dockerfile
```dockerfile
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . /build

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release
RUN cmake --install build --prefix /usr/local

WORKDIR /workspace
COPY .env /workspace/.env

EXPOSE 8080
CMD ["/usr/local/bin/OrderBookService"]
```

### Docker Compose
```yaml
version: '3.8'
services:
  lob-engine:
    build: .
    ports:
      - "8080:8080"
    environment:
      - MEMORY_POOL_SIZE=1000000
      - ENABLE_METRICS=ON
    volumes:
      - ./config:/workspace/config
      - ./data:/workspace/data
    restart: unless-stopped
```

### Kubernetes Deployment
```yaml
# k8s/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: lob-engine
spec:
  replicas: 3
  selector:
    matchLabels:
      app: lob-engine
  template:
    metadata:
      labels:
        app: lob-engine
    spec:
      containers:
      - name: lob-engine
        image: lob-engine:latest
        ports:
        - containerPort: 8080
        env:
        - name: MEMORY_POOL_SIZE
          value: "1000000"
        - name: ENABLE_METRICS
          value: "ON"
        resources:
          limits:
            cpu: "4"
            memory: "2Gi"
          requests:
            cpu: "2"
            memory: "1Gi"
```

## Performance Optimization

### CPU Optimization
```bash
# Set CPU affinity
taskset -c 0-3 ./build/bin/OrderBookService

# Use real-time priority (Linux)
chrt -f 50 ./build/bin/OrderBookService

# Disable CPU frequency scaling
cpupower frequency-set -g performance
```

### Memory Optimization
```bash
# Lock memory pages (prevent swapping)
mlockall  # In application code or use mlockall()

# Use huge pages
echo 1024 > /proc/sys/vm/nr_hugepages
```

### Network Optimization (if applicable)
```bash
# Set network buffer sizes
sysctl -w net.core.rmem_max=16777216
sysctl -w net.core.wmem_max=16777216

# Disable Nagle's algorithm
# In application code: setsockopt(TCP_NODELAY)
```

## Monitoring

### System Monitoring
```bash
# CPU monitoring
top -p $(pgrep OrderBookService)

# Memory monitoring
vmstat 1
free -h

# I/O monitoring
iostat -x 1

# Network monitoring
iftop
```

### Application Monitoring
```bash
# Enable metrics
export ENABLE_METRICS=ON

# Expose Prometheus metrics
# Access at http://localhost:9090/metrics
```

### Log Monitoring
```bash
# View logs
journalctl -u lob-engine -f  # systemd
tail -f /var/log/lob-engine.log  # file-based

# Rotate logs
logrotate /etc/logrotate.d/lob-engine
```

## High Availability

### Load Balancing
```nginx
# nginx.conf
upstream lob_engine {
    least_conn;
    server 10.0.0.1:8080;
    server 10.0.0.2:8080;
    server 10.0.0.3:8080;
}

server {
    listen 80;
    location / {
        proxy_pass http://lob_engine;
    }
}
```

### Failover
```bash
# Use keepalived for VIP failover
# Configure on multiple servers
apt-get install keepalived
```

### Data Replication
- Implement order book state replication
- Use message queues for event streaming
- Consider database for persistence

## Security

### Network Security
```bash
# Configure firewall
ufw allow 8080/tcp
ufw enable

# Use TLS for network communication
# Generate certificates
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365
```

### Application Security
```bash
# Run as non-root user
useradd -r -s /bin/false lob-engine
chown -R lob-engine:lob-engine /opt/lob-engine

# Set file permissions
chmod 750 /opt/lob-engine/bin
chmod 640 /etc/lob-engine/env
```

### Access Control
- Implement authentication for service deployment
- Use API keys or OAuth tokens
- Implement rate limiting
- Audit access logs

## Backup and Recovery

### Configuration Backup
```bash
# Backup configuration
tar -czf lob-engine-config-$(date +%Y%m%d).tar.gz /etc/lob-engine

# Restore configuration
tar -xzf lob-engine-config-20240803.tar.gz -C /
```

### State Backup
```bash
# Backup order book state
# Implement snapshot functionality
# Save to durable storage
```

### Disaster Recovery
- Document recovery procedures
- Test recovery procedures regularly
- Maintain off-site backups
- Consider geographic distribution

## Troubleshooting Deployment

### Service Won't Start
```bash
# Check logs
journalctl -u lob-engine -n 50

# Check configuration
cat /etc/lob-engine/env

# Check dependencies
ldd /opt/lob-engine/bin/OrderBookService
```

### Performance Issues
```bash
# Check system resources
top
vmstat
iostat

# Check application metrics
curl http://localhost:9090/metrics
```

### Network Issues
```bash
# Check connectivity
netstat -tlnp | grep 8080

# Check firewall
ufw status

# Test endpoint
curl http://localhost:8080/health
```

## Maintenance

### Updates
```bash
# Stop service
sudo systemctl stop lob-engine

# Backup current version
cp -r /opt/lob-engine /opt/lob-engine.backup

# Install new version
cmake --install build --prefix /opt/lob-engine

# Start service
sudo systemctl start lob-engine
```

### Monitoring Alerts
- Set up alerts for:
  - High CPU usage (>80%)
  - High memory usage (>80%)
  - High latency (>100ns p99)
  - Service downtime

### Regular Tasks
- Review logs weekly
- Check performance metrics
- Update dependencies monthly
- Security audit quarterly

## Scaling

### Vertical Scaling
- Increase CPU cores
- Add more memory
- Use faster storage (NVMe)
- Optimize CPU affinity

### Horizontal Scaling
- Add more instances
- Use load balancer
- Implement state sharing
- Consider distributed architecture

### Database Scaling
- Use read replicas for queries
- Partition by symbol
- Use caching layer
- Optimize database queries