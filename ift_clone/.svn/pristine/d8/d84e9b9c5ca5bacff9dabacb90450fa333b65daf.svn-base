import sys, os

import matplotlib.pyplot as plt
import numpy as np

def plot_cmc_curve(ranks, output_dir):
    tics = [i + 1 for i in range(len(ranks))]
    fig, ax = plt.subplots(figsize = (15,6))
    ax.plot(tics, ranks, linewidth = 3)
    ax.grid(True)
    ax.set_title("Cumulative Match Characteristic", fontsize = 18, fontweight = 'bold')
    ax.set_xlabel("Rank", fontsize = 18, fontweight = 'bold')
    ax.set_ylabel("Recognition rate (%)", fontsize = 18, fontweight = 'bold')
    ax.tick_params(axis = 'both', labelsize = 20)
    ax.set_ylim([0,100])

    plt.savefig(f'{output_dir}/identification_report/cmc.png')


def main():
    if (len(sys.argv) != 4):
        print("FPIdentificationReport usage: <P1> <P2> <P3>")
        print("P1: folder with identification results")
        print("P2: max CMC K-rank")
        print("P3: output folder with identification report")
        exit()
    
    input_folder = sys.argv[1]
    max_k_rank   = int(sys.argv[2])
    output_dir   = sys.argv[3] + '/'

    ranks = np.zeros(max_k_rank)
    
    

    for latent_result in os.listdir(input_folder):
        try:
            fp = open(f'{input_folder}/{latent_result}/scores.txt', 'r')
        except FileNotFoundError:
            fp = open(f'{input_folder}/{latent_result}/score.txt', 'r')
            
        lines  = fp.readlines()
        labels = np.array([int(item.split()[0].split('-')[0] == item.split()[1].split('-')[0]) for item in lines])
        
        if np.any(labels):
            first_occurance = np.argmax(labels)
        else:
            first_occurance = max_k_rank
        for j in range(first_occurance, max_k_rank):
            ranks[j] += 1
    
    print('Rank 1:    {} ({:.2f}%)'.format(ranks[0], ranks[0] * 100 / len(os.listdir(input_folder))))
    # print('Rank 2:    {} ({:.2f}%)'.format(ranks[1], ranks[1] * 100 / len(os.listdir(input_folder))))
    # print('Rank 3:    {} ({:.2f}%)'.format(ranks[2], ranks[2] * 100 / len(os.listdir(input_folder))))
    # print('Rank 10:   {} ({:.2f}%)'.format(ranks[9], ranks[9] * 100 / len(os.listdir(input_folder))))
    # print('Rank 20:   {} ({:.2f}%)'.format(ranks[19], ranks[19] * 100 / len(os.listdir(input_folder))))
    # print('Rank 30:   {} ({:.2f}%)'.format(ranks[29], ranks[29] * 100 / len(os.listdir(input_folder))))
    # print('Rank 100:  {} ({:.2f}%)'.format(ranks[99], ranks[99] * 100 / len(os.listdir(input_folder))))
    # print('Rank 1000: {} ({:.2f}%)'.format(ranks[999], ranks[999] * 100 / len(os.listdir(input_folder))))


    os.makedirs(output_dir + 'identification_report/', exist_ok = True)
    ranks = ranks / len(os.listdir(input_folder)) * 100



    plot_cmc_curve(ranks, output_dir)
    fp.close()

    

if __name__ == '__main__':
    main()