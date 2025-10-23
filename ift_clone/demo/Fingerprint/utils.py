def read_mnt_file(mnt_file):
    with open(mnt_file, 'r') as fp:
        mnts = fp.readlines()[2:]
    mnts = [tuple([i for i in item.strip().replace('   ', ' ').replace('  ', ' ').split(' ')]) for item in mnts]
    mnts = [(int(item[0]), int(item[1]), float(item[2])) for item in mnts] # transforming data types
    return mnts

def read_match_file(match_file):
    with open(match_file, 'r') as fp:
        header = fp.readline()
        header = header.strip().split()
        score, n_matches = float(header[0]), int(header[1])
        matches = fp.readlines()
    matches = [tuple([i for i in item.strip().split(' ')]) for item in matches]

    return score, n_matches, matches


def parse_minutia_match_filename(match_file):
    match_file = match_file.split('/')[-1]
    match_file = match_file.replace('.txt', '')
    latent_name, reference_name = match_file.split('_')

    return latent_name, reference_name