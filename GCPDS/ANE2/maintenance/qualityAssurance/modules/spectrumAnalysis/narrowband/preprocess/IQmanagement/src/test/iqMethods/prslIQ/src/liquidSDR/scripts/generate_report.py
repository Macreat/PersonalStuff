import json
import os

# generate_report.py: Create HTML dashboard from benchmark JSON

RESULTS_FILE = 'results/metrics_liquid.json'
OUTPUT_HTML = 'web/index.html'

def generate():
    if not os.path.exists(RESULTS_FILE):
        print(f"No results found at {RESULTS_FILE}")
        return

    with open(RESULTS_FILE, 'r') as f:
        data = json.load(f)

    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Liquid-DSP IQ Benchmark Report</title>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        <style>
            body {{ font-family: sans-serif; margin: 40px; background: #f4f4f9; }}
            .container {{ max-width: 800px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }}
            h1 {{ color: #333; }}
            table {{ width: 100%; border-collapse: collapse; margin-top: 20px; }}
            th, td {{ border: 1px solid #ddd; padding: 12px; text-align: left; }}
            th {{ background: #f8f8f8; }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Liquid-DSP IQ Benchmark Report</h1>
            <table>
                <tr><th>Metric</th><th>Value</th></tr>
                <tr><td>Throughput (SPS)</td><td>{data['throughput_sps']:,.2f}</td></tr>
                <tr><td>Total Time (ms)</td><td>{data['total_ms']:.2f}</td></tr>
                <tr><td>Peak RSS (MB)</td><td>{data['rss_peak_mb']:.2f}</td></tr>
                <tr><td>Samples Processed</td><td>{data['samples_total']:,}</td></tr>
            </table>
            
            <canvas id="timeChart" width="400" height="200"></canvas>
            <script>
                const ctx = document.getElementById('timeChart').getContext('2d');
                new Chart(ctx, {{
                    type: 'bar',
                    data: {{
                        labels: ['Load', 'Convert', 'Preprocess', 'Welch', 'Metric'],
                        datasets: [{{
                            label: 'Time (ms)',
                            data: [{data['load_ms']}, {data['convert_ms']}, {data['preprocess_ms']}, {data['welch_ms']}, {data['metric_ms']}],
                            backgroundColor: 'rgba(54, 162, 235, 0.5)'
                        }}]
                    }}
                }});
            </script>
        </div>
    </body>
    </html>
    """

    with open(OUTPUT_HTML, 'w') as f:
        f.write(html_content)
    
    print(f"Report generated: {OUTPUT_HTML}")

if __name__ == "__main__":
    generate()
