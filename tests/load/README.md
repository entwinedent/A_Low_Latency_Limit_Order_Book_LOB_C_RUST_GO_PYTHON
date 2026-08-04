# Load Testing

This directory contains load testing scripts and configurations for stress testing the Low-Latency Order Book Engine under high throughput conditions.

## Load Test Scenarios

### Throughput Tests
- **Maximum Throughput**: Determine maximum orders/second
- **Sustained Throughput**: Verify performance over extended periods
- **Burst Handling**: Test handling of order bursts
- **Multi-Symbol Scaling**: Test scaling with multiple symbols

### Latency Tests
- **Latency Distribution**: Measure latency percentiles (p50, p90, p99, p99.9)
- **Latency Under Load**: Measure latency at various load levels
- **Latency Spikes**: Detect and analyze latency spikes
- **Latency Stability**: Verify consistent latency over time

### Resource Tests
- **Memory Usage**: Monitor memory consumption under load
- **CPU Usage**: Monitor CPU utilization under load
- **Network Usage**: Monitor network bandwidth (if applicable)
- **Disk I/O**: Monitor disk operations (if applicable)

### Stability Tests
- **Long-Running**: Test stability over extended periods (24+ hours)
- **Memory Leak Detection**: Verify no memory leaks over time
- **Recovery Testing**: Test recovery from overload conditions
- **Graceful Degradation**: Test behavior under resource constraints

## Running Load Tests

### Quick Load Test
```bash
# Run basic load test
python tests/load/run_load_test.py --duration 60 --rate 1000000
```

### Comprehensive Load Test
```bash
# Run comprehensive load test suite
python tests/load/run_comprehensive_load.py --config tests/load/config/default.yaml
```

### Specific Scenarios
```bash
# Throughput test
python tests/load/scenarios/throughput.py --target 10000000

# Latency test
python tests/load/scenarios/latency.py --duration 300

# Stability test
python tests/load/scenarios/stability.py --duration 86400
```

## Load Test Configuration

### YAML Configuration
```yaml
# tests/load/config/default.yaml
test:
  duration: 300          # Test duration in seconds
  warmup: 10            # Warmup period in seconds
  cooldown: 10          # Cooldown period in seconds

workload:
  order_rate: 1000000   # Orders per second
  order_types:
    - limit: 0.7        # 70% limit orders
    - market: 0.2       # 20% market orders
    - cancel: 0.1       # 10% cancel orders
  
  symbols:
    - count: 10         # Number of symbols
    - distribution: uniform  # Distribution of orders across symbols

performance:
  targets:
    throughput: 10000000    # Target orders/second
    latency_p99: 100        # Target p99 latency (ns)
    memory_max: 200         # Max memory (MB)
    cpu_max: 80             # Max CPU usage (%)

monitoring:
  interval: 1               # Monitoring interval (seconds)
  metrics:
    - throughput
    - latency
    - memory
    - cpu
```

## Load Test Tools

### Custom Load Test Runner
```python
# tests/load/run_load_test.py
import argparse
from load_test import LoadTestRunner

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--duration', type=int, default=60)
    parser.add_argument('--rate', type=int, default=1000000)
    parser.add_argument('--config', type=str, default=None)
    args = parser.parse_args()
    
    runner = LoadTestRunner(args.config)
    runner.run(duration=args.duration, rate=args.rate)

if __name__ == '__main__':
    main()
```

### Industry Tools Integration
- **JMeter**: HTTP load testing for service deployment
- **Locust**: Python-based load testing framework
- **k6**: Modern load testing tool
- **wrk**: HTTP benchmarking tool

## Performance Targets

### Throughput Targets
| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| Orders/Second | 10M+ | TBD | TBD |
| Matches/Second | 5M+ | TBD | TBD |
| Multi-Symbol | 1M/symbol | TBD | TBD |

### Latency Targets
| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| p50 Latency | <20 ns | TBD | TBD |
| p90 Latency | <30 ns | TBD | TBD |
| p99 Latency | <50 ns | TBD | TBD |
| p99.9 Latency | <100 ns | TBD | TBD |

### Resource Targets
| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| Memory Usage | <200 MB | TBD | TBD |
| CPU Usage | <80% | TBD | TBD |
| Memory Leaks | 0 | TBD | TBD |

## Monitoring During Load Tests

### Real-time Monitoring
```bash
# Start monitoring
python tests/load/monitor.py --interval 1

# View metrics
# Throughput, latency, memory, CPU
```

### Post-Test Analysis
```bash
# Generate report
python tests/load/analyze.py --input results/

# Generate graphs
python tests/load/graphs.py --input results/
```

## Load Test Patterns

### Market Making Pattern
```python
# Continuously place bid/ask orders
# Maintain 2-sided market
# Adjust spread based on market conditions
```

### High-Frequency Trading Pattern
```python
# Rapid order submission and cancellation
# Short-term positions
# High order-to-trade ratio
```

### Retail Trading Pattern
```python
# Lower order rate
- Larger order sizes
- Longer holding periods
- Market orders more common
```

### Institutional Trading Pattern
```python
# Large order sizes
- Algorithmic execution
- Time-weighted average price (TWAP)
- Volume-weighted average price (VWAP)
```

## CI/CD Integration

### Load Tests in CI
```yaml
# .github/workflows/load.yml
name: Load Tests
on:
  schedule:
    - cron: '0 2 * * *'  # Daily at 2 AM
  push:
    branches: [main]

jobs:
  load:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
          cmake --build build --config Release
      - name: Load Tests
        run: |
          python tests/load/run_comprehensive_load.py \
            --config tests/load/config/ci.yaml
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: load-test-results
          path: artifacts/load/
```

## Performance Regression Detection

### Baseline Comparison
```python
# Compare current results with baseline
python tests/load/compare.py --current results/ --baseline baseline/

# Alert if regression detected
# - Throughput degradation >10%
# - Latency increase >20%
# - Memory increase >15%
```

### Trend Analysis
```python
# Track performance over time
python tests/load/trend.py --history results/history/

# Generate trend graphs
# - Throughput over time
# - Latency over time
# - Resource usage over time
```

## Troubleshooting Load Tests

### Performance Degradation
- Check system resources (CPU, memory, disk)
- Verify build configuration (Release mode)
- Check for background processes
- Monitor thermal throttling

### Test Flakiness
- Ensure proper warmup and cooldown periods
- Check for resource contention
- Verify test isolation
- Add retry logic for transient failures

### Resource Exhaustion
- Reduce load test parameters
- Check for memory leaks
- Monitor system limits
- Increase system resources if needed

## Best Practices

### Test Design
1. **Realistic**: Simulate real-world usage patterns
2. **Gradual**: Start with low load, increase gradually
3. **Sustained**: Include long-running tests
4. **Comprehensive**: Test various scenarios

### Test Execution
1. **Warmup**: Always include warmup period
2. **Monitoring**: Monitor throughout test
3. **Isolation**: Run on dedicated resources
4. **Repeatability**: Ensure consistent results

### Result Analysis
1. **Baseline**: Compare against established baselines
2. **Trends**: Look for performance trends over time
3. **Outliers**: Investigate performance outliers
4. **Context**: Consider system conditions

## Writing New Load Tests

### Template
```python
# tests/load/scenarios/custom_scenario.py
from load_test import LoadTestScenario

class CustomScenario(LoadTestScenario):
    def setup(self):
        # Initialize scenario
        self.order_book = lob.OrderBook()
    
    def generate_order(self):
        # Generate order for this scenario
        return lob.Order(...)
    
    def validate(self, results):
        # Validate results
        assert results.throughput > self.target_throughput
        assert results.latency_p99 < self.target_latency

if __name__ == '__main__':
    scenario = CustomScenario()
    scenario.run()
```

## References

- **Load Testing Best Practices**: https://en.wikipedia.org/wiki/Load_testing
- **Performance Testing**: https://www.guru99.com/performance-testing.html
- **System Performance**: https://www.brendangregg.com/