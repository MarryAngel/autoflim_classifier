import sys, os

import matplotlib.pyplot as plt
from sklearn.metrics import precision_recall_curve, roc_curve, roc_auc_score
from sklearn.metrics import recall_score, precision_score, accuracy_score


import numpy as np

def accuracy(impostor_results, genuine_results, threshold):
    imp_results = np.array(impostor_results)
    gen_results = np.array(genuine_results)


    return (len(imp_results[imp_results < threshold]) + len(gen_results[gen_results >= threshold])) / (len(imp_results) + len(gen_results))

def best_threshold(impostor_results, genuine_results):
    # we assume that impostor results are generally lower than genuine
    imp_results = impostor_results.copy()
    imp_results.sort()

    thresholds = imp_results[::2]
    accs       = []

    for thresh in thresholds:
        accs.append(accuracy(impostor_results, genuine_results, thresh))
    
    best_index = np.argmax(accs)

    return accs[best_index], thresholds[best_index]


def plot_precision_recall(precision, recall, output_dir):
    fig, ax = plt.subplots(figsize = (15,6))
    ax.plot(recall, precision, linewidth = 3)
    ax.grid(True)
    ax.set_title("Precision-Recall Curve", fontsize = 18, fontweight = 'bold')
    ax.set_xlabel("Recall", fontsize = 18, fontweight = 'bold')
    ax.set_ylabel("Precision", fontsize = 18, fontweight = 'bold')
    ax.tick_params(axis = 'both', labelsize = 20)

    plt.savefig(f'{output_dir}/verification_report/precision_recall.png')

def plot_ROC(false_positive_rate, true_positive_rate, roc_score, output_dir):
    fig, ax = plt.subplots(figsize = (15,6))
    ax.plot([0, 1], [0, 1], "k--", label="chance level (AUC = 0.5)", linewidth = 3)
    ax.plot(false_positive_rate, true_positive_rate, label = 'classifier', linewidth = 3)
    ax.legend()
    ax.grid(True)
    ax.set_title("ROC Curve", fontsize = 18, fontweight = 'bold')
    ax.set_xlabel("False Positive Rate", fontsize = 18, fontweight = 'bold')
    ax.set_ylabel("True Positive Rate", fontsize = 18, fontweight = 'bold')
    ax.annotate("AUC Score: {:.2}".format(roc_score), (0.6, 0.2), fontsize = 16)
    ax.tick_params(axis = 'both', labelsize = 20)

    plt.savefig(f'{output_dir}/verification_report/ROC.png')

def get_accs_for_each_threshold(thresholds, scores, true_labels):
    accs = []
    for t in thresholds:
        correct = 0
        for i in range(len(scores)):
            if scores[i] > t:
                label = 1
            else:
                label = 0
            
            if (label == true_labels[i]):
                correct += 1
        
        accs.append(correct/len(scores))
    
    return accs

def plot_accuracy_thresholds(accuracys, thresholds, output_dir):
    fig, ax = plt.subplots(figsize = (15,6))
    ax.plot(thresholds, accuracys, linewidth = 3)
    ax.axvline(thresholds[np.argmax(accuracys)], ymin = 0, ymax = 1, linestyle = '--', color = 'red')
    ax.grid(True)
    ax.set_title("Accuracy versus Thresholds", fontsize = 18, fontweight = 'bold')
    ax.set_xlabel("Threshold", fontsize = 18, fontweight = 'bold')
    ax.set_ylabel("Accuracy", fontsize = 18, fontweight = 'bold')
    ax.tick_params(axis = 'both', labelsize = 20)

    plt.savefig(f'{output_dir}/verification_report/accs_thresh.png')


def main():
    if (len(sys.argv) != 3):
        print("FPVerificationReport usage: <P1> <P2>")
        print("P1: scores file (.txt)")
        print("P2: output folder with verification report")
        exit()
    
    output_dir = sys.argv[2] + '/'
    os.makedirs(output_dir + 'verification_report/', exist_ok = True)

    # Reading scores file
    scores_file = sys.argv[1]
    with open(scores_file) as fp:
        lines  = fp.readlines()
        scores = np.array([float(item.strip().split()[-1]) for item in lines])
        labels = np.array([item.split()[0].split('-')[0] == item.split()[1].split('-')[0] for item in lines])

    # Classification metrics
    precision, recall, thresholds = precision_recall_curve(labels, scores)
    fpr, tpr, _         = roc_curve(labels, scores, pos_label=1)
    roc_score           = roc_auc_score(labels, scores)
    accs                = get_accs_for_each_threshold(thresholds, scores, labels)

    y_hat = np.where(scores > thresholds[np.argmax(accs)], 1, 0)
    accuracy  = accuracy_score(labels, y_hat)
    precision = precision_score(labels, y_hat)
    recall    = recall_score(labels, y_hat)

    print("Accuracy: {:>6.2f}%".format(accuracy * 100))
    print("Precision: {:.2f}%".format(precision * 100))
    print("Recall: {:>8.2f}%".format(recall * 100))

    # Graphics

    # Precision-recall curve
    plot_precision_recall(precision, recall, output_dir)

    # ROC curve
    plot_ROC(fpr, tpr, roc_score, output_dir)

    # Accuracy vs thresholds
    plot_accuracy_thresholds(accs, thresholds, output_dir)

if __name__ == '__main__':
    main()