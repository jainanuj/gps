import os

current_dir = "./"
test_run_root = "test_runs/"
sizedirs = "/"
mdpdirs = "mdps/"
partitiondirs = "partitions/"
opdir = "./results/outputs/"
statdir ="./results/stats/"
statfile = "statsummary/summarytable.txt"

pre_pend_path = "../tests/"

startdir = current_dir + test_run_root
statfileptr = open(statdir + statfile, "a")
statfileptr.write("Mdp_size, part_dims, part_avg_bytes, #updates, time_taken, Iter_Time, Queue_Time \n")
statfileptr.close()

for size in os.listdir(startdir):
    for mdp in os.listdir(startdir + size + '/' + mdpdirs):
        for part in os.listdir(startdir + size + '/' + partitiondirs):
            mdpPath = current_dir + test_run_root + size + '/' + mdpdirs + mdp
            partPath = current_dir + test_run_root + size + '/' + partitiondirs + part
#            print "MDP = ", mdpPath, " Part = ", partPath
            opfile = "results" + "_" + mdp + "_" + size + "_" + part
            cmd_to_exec = "../solve/gps --epsilon=0.0001  --odcd_cache_fn_format=/tmp/odcd_cache-%d  --odcd_cache_size=-1 \
--mdp_fn=" + mdpPath + "  --stp_fn=" + partPath + "  --run_type=vi  --heat_metric=abs  --solver=r  \
--use_voting=0   --save_fn=" + opdir + opfile + " --stat_fn=" + statdir + statfile + "  --verbose=1 >" + statdir + opfile
            statfileptr = open(statdir + statfile, "a")
            statfileptr.write(mdp + '_' +  size + ', ' + part + ', ')
            statfileptr.close()
            print(cmd_to_exec)
            os.system(cmd_to_exec)
