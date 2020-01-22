import numpy as np
import re
import os
import shutil
import configparser
import matplotlib.pyplot as plt
from glob import glob
import datetime
import zipfile
import scipy.io as sio

# Script runs simulator in a parallel manner and creates plots in PDF
# Configure simulator only by omnetpp.ini !

# Translates omnetpp.ini for the pdf report
# WARNING: Edit run_simulation() to specify the path of omnetpp bin

class Result:
    def __init__(self, a, b, c, exper, meas, scen):
        self.plotTitle = a
        self.y_axis = b
        self.tmp = 0
        self.filename_ending_string = c
        self.final = np.zeros((scen.__len__(), exper.__len__(), meas.__len__()))


def plot_json(results_file_path):

    results_set = sio.loadmat(results_file_path)

    x_axis_data = results_set["details"][0][1][0][0][0][0]
    experiment_str = results_set["details"][0][2][0][0][0][0]
    x_axis_label = results_set["details"][0][0][0][0][0][0]

    for x in results_set:
        if x != "details" and x != "__header__" and x != "__version__" and x != "__globals__":

            # print(results_set[x][0][0][3][0])

            plot_title = results_set[x][0][0][0][0]
            y_axis_label = results_set[x][0][0][1][0]
            filename_ending_string = results_set[x][0][0][3][0]
            
            fig = plt.figure(num=plot_title)
            for i in range(0, 2):
                y_axis_data = results_set[x][0][0][4][i][0]
                plot_name = results_set["details"][0][3][0][0][i][0]
                plt.plot(x_axis_data, y_axis_data, label=plot_name)

            # plt.axis([N[0], N[-1], ])
            plt.title(plot_title)
            plt.xlabel(x_axis_label)
            plt.ylabel(y_axis_label)
            plt.grid(True)
            plt.xticks(x_axis_data)
            # plt.yticks(eed)
            plt.legend()
            filename = os.path.dirname(results_file_path) + "/" + experiment_str + filename_ending_string
            print(filename)
            fig.savefig(filename)


def generate_report():

    lexicon = {
        'network.olt.schedulerus.r_f': 'Fixed ($R_f$)',
        'network.olt.schedulerus.r_a': 'Assured ($R_a$)',
        'network.olt.schedulerus.r_m': 'Maximum ($R_m$)',
        'network.olt.twdmchannels': 'Number of channels',
        'description': 'Description of experiment',
        'network.onus': 'Number of ONUs',
        'network.onu[*].allocid[*].appcbr.packetsize': 'Packet length of CBR app',
        'network.onu[*].allocid[*].appcbr.rate': 'Data rate of CBR app for each AllocID',
        'network.onu[*].distance': "ONU's distance from OLT",
        'experiment-label': 'Name of experiment',
        'network.onu[*].allocid[*].queue.capacity': 'Queue capacity',
        'network.onu[*].numallocids': 'AllocIDs per ONU',
        'network.olt.schedulerus.dwbamode': 'DWBA modes'
    }

    def purify_latex(val):
        val = val.replace("${rate=", "")
        val = val.replace("${ONUs=", "")
        val = val.replace("${dwba=", "")
        val = val.replace('}', '')
        return val

    def value_purifier(val):
        if lexicon.__contains__(val):
            return lexicon.get(val)
        else:
            return val

    tex_full_path = folderResults + "/configuration.tex"
    tex = open(tex_full_path, 'w+')

    tex.write("\\documentclass{article}\n\n")
    tex.write("\\usepackage[textwidth=19cm,textheight=20cm]{geometry}\n")
    tex.write("\\begin{document}\n")

    tex.write("\\section*{\\texttt{[" + timestamp.replace("_", "\\_") + "]} Simulation configuration report}\n")

    tex.write("\\subsection*{General parameters}\n")
    tex.write("\\begin{tabular}{ll}\n")
    tex.write("Simulation time " + "& " + str(simulation_time) + " sec" + "\\\\ \n")
    tex.write("Replications " + "& " + str(repetitions) + "\\\\ \n")
    tex.write("\\end{tabular}\n\n")

    tmp = omnetpp_ini.items("General")
    tex.write("\\subsection*{Global parameters}\n")
    tex.write("\\begin{tabular}{ll}\n")
    for value in tmp:
        if value[0] == 'sim-time-limit' or value[0] == 'repeat' or value[0] == 'debug-on-errors' or \
                value[0] == 'network' or value[0] == 'eventlog-message-detail-pattern':
            continue
        else:
            tex.write(value_purifier(value[0]) + ' & ' + purify_latex(value[1]) + "\\\\ \n")
    tex.write("\\end{tabular}\n\n")

    tmp = omnetpp_ini.items("Config " + experiment_string)
    tex.write("\\subsection*{Experiment-specific parameters}\n")
    tex.write("\\begin{tabular}{lp{7cm}}\n")
    for value in tmp:
        tex.write(value_purifier(value[0]) + ' & ' + purify_latex(value[1]) + "\\\\ \n")
    tex.write("\\end{tabular}\n\n")

    tex.write("\\end{document}")

    tex.close()

    os.system("pdflatex -output-directory=" + folderResults + " " + tex_full_path)


def tide_up():
    # Tiding up
    os.chdir(folderResults)

    listing = glob("configuration.*")
    for filename in listing:
        if filename.endswith(".pdf"):
            continue
        else:
            os.remove(filename)

    z = zipfile.ZipFile("sca_files.zip", "w")
    listing = glob("*.sca")
    for filename in listing:
        z.write(filename)
        os.remove(filename)
    z.close()
    z = zipfile.ZipFile("out_files.zip", "w")
    listing = glob("*.out")
    for filename in listing:
        z.write(filename)
        os.remove(filename)
    z.close()
    z = zipfile.ZipFile("vec_files.zip", "w")
    listing = glob("../*.vec")
    for filename in listing:
        z.write(filename)
        os.remove(filename)
    z.close()


def export_results():
    def unit_to_number(mixed):
        value = ''
        for letter in mixed:
            if letter.isnumeric():
                value = value.__add__(letter)
        return value
    # First of all, read all files to define x-axis (measurements) and number of plots (experiments) in the same figure.
    measurements = set([])
    experiments = set([])
    scenarios = set([])

    sca_listing = glob(folderResults + "/*.sca")
    vec_listing = glob(folderResults + "/*.vec")

    for sca in sca_listing:  # Read each results file to see how many measurements there are
        f = open(sca)
        line = f.readline()
        while line:
            line = line.split()
            if line:
                if len(line) == 3:
                    if line[1] == measurement_string:  # Measurement string
                        measurements.add(float(unit_to_number(line[2])))
                    elif line[1] == "configname":      # Experiment is the config label
                        experiments.add(line[2])
                    elif line[1] == "dwba":            # scenario is the dwba algorithm
                        scenarios.add(line[2])
            line = f.readline()
            
    measurements = sorted(measurements)
    experiments = list(experiments)
    scenarios = sorted(scenarios)

    # After that, you can pre-allocate the lists with the results.
    results_set = {
        "eed":              Result("Mean end-to-end delay", "EED (msec)", "_eed.pdf", experiments, measurements,
                                   scenarios),
        "goodput":          Result("Mean network goodput", "Goodput (Mbps)", "_goodput.pdf", experiments, measurements,
                                   scenarios),
        "queueing_delay":   Result("Mean queueing delay", "Queueing delay (msec)", "_queueing_delay.pdf", experiments,
                                   measurements, scenarios),
        "packets_received": Result("Total packets received", "Packets received", "_packets_received.pdf", experiments,
                                   measurements, scenarios),
        "drop_rate":        Result("Mean drop rate", "Drop rate", "_drop_rate.pdf", experiments, measurements,
                                   scenarios),
        "utilization":      Result("Utilization", "Utilization", "_utilization.pdf", experiments, measurements,
                                   scenarios),
        "delay_fairness":   Result("Delay fairness", "Fairness", "_delayfairness.pdf", experiments, measurements,
                                   scenarios)

    }

    # Read again
    for sca in sca_listing:  # Read each results file
        tmp_measurement = tmp_experiment = tmp_scenario = 0

        f = open(sca)
        line = f.readline()

        while line:  # Read scalar results of a single replication
            line = line.split()
            if line:
                if len(line) == 3:
                    if line[1] == measurement_string:  # What measurement has been conducted?
                        tmp_measurement = float(unit_to_number(line[2]))
                    elif line[1] == "configname":  # What experiment has been conducted?
                        tmp_experiment = line[2]
                    elif line[1] == "dwba":  # what dwba has been applied?
                        tmp_scenario = line[2]
                elif len(line) == 4:
                    if line[2] == "bytesReceived:sum":
                        results_set["goodput"].tmp = (float(line[3]) * 0.000008)/simulation_time  # conversion to Mbps
                        # tmp_sum_bytes = float(line[3]) * 0.000008  # conversion to Mbps
                    elif line[2] == "eed:mean":
                        results_set["eed"].tmp = float(line[3]) * 1000  # conversion to msec
                        # tmp_eed = float(line[3]) * 1000  # conversion to msec
                    elif line[2] == "queueingDelay:mean":
                        results_set["queueing_delay"].tmp = float(line[3]) * 1000  # conversion to msec
                        # tmp_queueing_delay = float(line[3]) * 1000  # conversion to msec
                    elif line[2] == "packetsNum:count":
                        results_set["packets_received"].tmp = int(line[3])
                    elif line[2] == "utilization:mean":
                        results_set["utilization"].tmp = float(line[3])
                    elif line[2] == "dropRate:last":
                        if line[3] == "nan":
                            results_set["drop_rate"].tmp = 0
                        else:
                            results_set["drop_rate"].tmp = float(line[3])

            line = f.readline()

        # Results that belong to the same measurement and experiment must be averaged by the number of replications
        for x in results_set:
            results_set[x].final[scenarios.index(tmp_scenario)][experiments.index(tmp_experiment)][measurements.index(tmp_measurement)] += results_set[x].tmp / repetitions

    results_set.update({"details": ({"x_axis": xAxis}, {"measurements": measurements}, {"experiments": experiments}, scenarios_strings)})
    result_file_path_string = folderResults + "/" + experiment_string + "_results"
    sio.savemat(result_file_path_string, results_set)

    return result_file_path_string


''''
    for vec in vec_listing: # Read scalar results
        tmp_measurement = tmp_experiment = tmp_scenario = 0
        found_vector_1 = False
        found_vector_2 = False

        f = open(vec)
        line = f.readline()

        while line:  # Read vector results of a single replication
            line = line.split()
            if line:
                if len(line) == 3
'''


def run_simulation():
    # Run the .ini file.

    if os.name == 'nt':
        # For windows, add bins and libs to the PATH. os.environ["PATH"] += os.pathsep + app_path
        os.environ["PATH"] += os.pathsep + "D:\\omnetpp-5.5\\tools\\win64\\usr\\bin" + os.pathsep + \
                              "D:\\omnetpp-5.5\\tools\\win64\\mingw64\\bin" + os.pathsep + "D:\\omnetpp-5.5\\bin"
        command = "opp_runall ./src/ngpon2.exe -c " + experiment_string + \
                  " -n 'simulations;src' -f simulations/omnetpp.ini"
    else:
        os.environ['PATH'] += os.pathsep + '/home/cdal/omnetpp-5.5.1/bin'
        command = "opp_runall ./src/ngpon2 -c " + experiment_string + " -n simulations:src -f ./simulations/omnetpp.ini"
    print(command)
    os.system(command)

    # Move all .sca and .out files to results folder
    tmp_listing = glob("simulations/results/*.sca")
    for file in tmp_listing:
        shutil.move(file, folderResults)
    tmp_listing = glob("simulations/results/*.out")
    for file in tmp_listing:
        shutil.move(file, folderResults)


if __name__ == '__main__':

    # experiment_string is a section in omnetpp.ini that defines a simulation scenario
    experiment_string = 'GradualIncreaseONUs'  # 'GradualIncreaseONUs' 'GradualIncreaseLoad'

    # measurement_string is a key that has multiple values, each value defines an extra measurement and record in x-axis
    if experiment_string == "GradualIncreaseONUs":
        measurement_string = 'Network.onus'
        xAxis = 'ONUs'
    elif experiment_string == "GradualIncreaseLoad":
        measurement_string = 'Network.onu[*].allocId[*].appCbr.rate'
        xAxis = 'Load per AllocID (Mbps)'

    scenarios_strings = {
        "Scenario0": "Round-robin",
        "Scenario1": "Game theory"
    }

    omnetpp_ini = configparser.ConfigParser()
    omnetpp_ini.read("simulations/omnetpp.ini")

    repetitions = int(omnetpp_ini.get("General", "repeat"))  # How many times a measurement should be repeated
    simulation_time = float(re.sub("s", "", omnetpp_ini.get("General", 'sim-time-limit')))  # get duration of simulation
    timestamp = str(datetime.datetime.now().date()) + "_" + str(datetime.datetime.now().time().hour) + "-" + \
        str(datetime.datetime.now().time().minute)  # get current timestamp

    # folderResults = "simulations/results/test"  # Uncomment it to put results in a customized folder

    if 'folderResults' not in globals():
        # Create folder results. Any output will be moved there
        folderResults = "simulations/results/" + timestamp
        if not os.path.exists(folderResults):
            os.makedirs(folderResults)

    backup_path = os.environ["PATH"]
    run_simulation()  # Gives omnetpp.ini and experiment_string as parameters to opp_runall; fires up multiple sims
    os.environ["PATH"] = backup_path

    print("Parallel simulation done. Gathering results...")

    result_path = export_results()

    plot_json(result_path)

    # Reads .ini and generates report
    generate_report()

    tide_up()

    print("END OF SIMULATION")
