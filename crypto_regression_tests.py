from compare_traces import compare_traces
import os
import shutil
from git import Repo

commit_hash_baseline = "364771195a4751d4e5a"
commit_hash_hwmod    = "6fd978f0a9dc4d9b5cf"

#baseline_csv_dir   = os.path.join("csv", "baseline")

test_to_run = "dogfood"

def generate_logfile(commit_hash, test_type): 
        commit = repo.commit(commit_hash)
        
        print(f"Using commit {commit_hash} - {commit.message}")
        
        csv_dir = os.path.join(os.getcwd(), "csv", test_type)
        os.mkdir(csv_dir)

        git_cmd.checkout(commit_hash, "hw")
        git_cmd.checkout(commit_hash, "tests")
        
        if os.path.exists("build"):
            shutil.rmtree("build")
        
        os.mkdir("build")
        os.system('''
                    cd build
                    ../configure --xlen=64 --tooldir=$HOME/tools
                    make -s
                    cd runtime/rtlsim
                    CONFIGS="-DL1_DISABLE -DLMEM_DISABLE -DNUM_CORES=1 -DNUM_WARPS=1 -DNUM_THREADS=1 -DNUM_CLUSTER=1 -DNUM_SOCKETS=1" DEBUG=3 make      
                  ''')
        
        print(f"\nRunning {test_to_run} test\n")
        os.system(f'''
                    cd build/tests/regression/{test_to_run}
                    make run-rtlsim > run.log
                  ''')

        log_fullpath = os.path.join(os.getcwd(), csv_dir, test_type + ".log")
        csv_fullpath = os.path.join(os.getcwd(), csv_dir, test_type + ".csv")
    

        print(f"Copy log file to {log_fullpath}")
        
        shutil.copyfile(os.path.join("build", "tests", "regression", test_to_run, "run.log"), log_fullpath)
        
        os.system(f'''
                  python ci/trace_csv.py -t rtlsim {log_fullpath} -o {csv_fullpath}  
                  ''')


if __name__ == "__main__":
    try:
        repo    = Repo(os.getcwd())
        git_cmd = repo.git
    except:
            print("Git repository not found in current directory")


    print("Running Regression tests on hardware modifications........")
    
    if not os.path.exists(os.path.join(os.getcwd(), "csv")):
        os.mkdir(os.path.join(os.getcwd(), "csv"))


    baseline_csv_dir = os.path.join("csv", "baseline")
    if not os.path.exists(os.path.join(os.getcwd(), baseline_csv_dir)):
        print("Baseline csvs not found, regenerating...")
        
        generate_logfile(commit_hash_baseline, "baseline")
        #print(f"Currently in {os.getcwd()}")
    
    hw_csv_dir = os.path.join("csv", "hwmod")
    
    if os.path.exists(hw_csv_dir):
        shutil.rmtree(hw_csv_dir)
    
    print("Generating csvs for hardware modifications....")
    
    generate_logfile(commit_hash_hwmod, "hwmod")
    
    hw_csv_path       = os.path.join("csv", "hwmod", "hwmod.csv")
    baseline_csv_path = os.path.join("csv", "baseline", "baseline.csv")
    
    compare_traces(baseline_csv_path, hw_csv_path)

    #print(f"Using commit {commit_hash_baseline}")
