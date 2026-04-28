"""
Compare classification results between baseline and filter_reduction models.
Extract F1-score per class and generate comparison analysis.
"""
import os
import pandas as pd
import numpy as np
from sklearn.metrics import f1_score, precision_recall_fscore_support, confusion_matrix
import json

def read_classification_file(filepath):
    """Read classification results from CSV file"""
    data = []
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return None
    
    with open(filepath, 'r') as f:
        for line in f:
            parts = line.strip().split(';')
            if len(parts) >= 3:
                y_true = int(parts[1])
                y_pred = int(parts[2])
                data.append({'y_true': y_true, 'y_pred': y_pred})
    
    return pd.DataFrame(data)

def calculate_metrics_per_class(y_true, y_pred, num_classes=9):
    """Calculate precision, recall, F1 per class"""
    precision, recall, f1, support = precision_recall_fscore_support(
        y_true, y_pred, 
        labels=range(1, num_classes + 1),
        zero_division=0
    )
    
    return {
        'precision': precision,
        'recall': recall,
        'f1': f1,
        'support': support
    }

def generate_report(baseline_dir, filter_reduction_dir, dataset_name='eggs', num_classes=9):
    """Generate comparison report between baseline and filter_reduction"""
    
    results = {
        'dataset': dataset_name,
        'num_classes': num_classes,
        'splits': {},
        'summary': {}
    }
    
    for split in [1, 2, 3]:
        print(f"\n{'='*80}")
        print(f"Split {split}")
        print(f"{'='*80}")
        
        # Read baseline results
        baseline_file = os.path.join(
            baseline_dir, f'super50', f'split{split}', 'results',
            f'layer3_test{split}-classified-images.csv'
        )
        baseline_df = read_classification_file(baseline_file)
        
        # Read filter_reduction results
        filter_file = os.path.join(
            filter_reduction_dir, f'super50', f'split{split}', 'results',
            f'layer3_test{split}-classified-images.csv'
        )
        filter_df = read_classification_file(filter_file)
        
        if baseline_df is None or filter_df is None:
            print(f"Skipping split {split} - missing results")
            continue
        
        # Calculate metrics
        baseline_metrics = calculate_metrics_per_class(
            baseline_df['y_true'], baseline_df['y_pred'], num_classes
        )
        filter_metrics = calculate_metrics_per_class(
            filter_df['y_true'], filter_df['y_pred'], num_classes
        )
        
        # Store results
        results['splits'][f'split{split}'] = {
            'baseline': {
                'precision': baseline_metrics['precision'].tolist(),
                'recall': baseline_metrics['recall'].tolist(),
                'f1': baseline_metrics['f1'].tolist(),
                'support': baseline_metrics['support'].tolist(),
                'macro_f1': float(np.mean(baseline_metrics['f1'])),
                'weighted_f1': float(np.average(baseline_metrics['f1'], weights=baseline_metrics['support']))
            },
            'filter_reduction': {
                'precision': filter_metrics['precision'].tolist(),
                'recall': filter_metrics['recall'].tolist(),
                'f1': filter_metrics['f1'].tolist(),
                'support': filter_metrics['support'].tolist(),
                'macro_f1': float(np.mean(filter_metrics['f1'])),
                'weighted_f1': float(np.average(filter_metrics['f1'], weights=filter_metrics['support']))
            }
        }
        
        # Print per-class comparison
        print("\nPer-Class F1 Comparison:")
        print(f"{'Class':<8} {'Support':<10} {'Baseline F1':<15} {'Filter F1':<15} {'Diff':<10}")
        print("-" * 60)
        
        for i in range(num_classes):
            class_num = i + 1
            support = int(baseline_metrics['support'][i])
            baseline_f1 = baseline_metrics['f1'][i]
            filter_f1 = filter_metrics['f1'][i]
            diff = filter_f1 - baseline_f1
            
            print(f"{class_num:<8} {support:<10} {baseline_f1:<15.4f} {filter_f1:<15.4f} {diff:+.4f}")
        
        print("\nAggregate Metrics:")
        print(f"  Baseline - Macro F1: {results['splits'][f'split{split}']['baseline']['macro_f1']:.4f}, "
              f"Weighted F1: {results['splits'][f'split{split}']['baseline']['weighted_f1']:.4f}")
        print(f"  Filter   - Macro F1: {results['splits'][f'split{split}']['filter_reduction']['macro_f1']:.4f}, "
              f"Weighted F1: {results['splits'][f'split{split}']['filter_reduction']['weighted_f1']:.4f}")
    
    return results

def main():
    # Paths
    base_dir = os.path.abspath(os.path.dirname(__file__))
    extras_dir = os.path.join(base_dir, '..', 'extras', 'exp', 'eggs')
    
    baseline_dir = os.path.join(extras_dir, 'baseline')
    filter_reduction_dir = os.path.join(extras_dir, 'filter_reduction')
    
    output_file = os.path.join(extras_dir, 'comparison_analysis.json')
    
    print("Comparison Analysis: Baseline vs Filter Reduction")
    print(f"Baseline dir: {baseline_dir}")
    print(f"Filter reduction dir: {filter_reduction_dir}")
    
    # Check if directories exist
    if not os.path.exists(baseline_dir):
        print(f"\nERROR: Baseline directory not found: {baseline_dir}")
        print("Please run: python train_baseline_eggs.py")
        return
    
    if not os.path.exists(filter_reduction_dir):
        print(f"\nERROR: Filter reduction directory not found: {filter_reduction_dir}")
        print("Please run: python filter_reduction_eggs.py")
        return
    
    # Generate report
    results = generate_report(baseline_dir, filter_reduction_dir)
    
    # Save results to JSON
    os.makedirs(extras_dir, exist_ok=True)
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2)
    
    print(f"\n{'='*80}")
    print(f"Report saved to: {output_file}")
    print(f"{'='*80}\n")

if __name__ == '__main__':
    main()
