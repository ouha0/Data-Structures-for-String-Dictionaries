#!/usr/bin/awk -f

# Example of extracting each type of line
/SZX/ { node_size = $1 }
/NUX/ { non_unique += $1; count_nux++ }
/INX/ { insert_time += $1; count_inx++ }
/SRX/ { search_time += $1; count_srx++ }
/MUX/ { memory_usage += $1; count_mux++ }
/UKX/ { unique_keys += $1; count_ukx++ }
/KPX/ { keys_processed += $1; count_kpx++ }
/NNX/ { number_of_nodes += $1; count_nnx++ }
/NRX/ { node_ratio += $1; count_nrx++ }

# At the end of processing, print averages with formatting
END {
    print "Node Size: ", node_size
    print "Average Non-Unique Strings: ", (count_nux ? non_unique / count_nux : 0)
    printf "Average Insert Time: %.3f seconds\n", (count_inx ? insert_time / count_inx : 0)
    printf "Average Search Time: %.3f seconds\n", (count_srx ? search_time / count_srx : 0)
    print "Average Memory Usage: ", (count_mux ? memory_usage / count_mux : 0), "bytes"
    print "Average Unique Strings: ", (count_ukx ? unique_keys / count_ukx : 0)
    print "Average Keys Processed: ", (count_kpx ? keys_processed / count_kpx : 0)
    print "Average Number of Nodes: ", (count_nnx ? number_of_nodes / count_nnx : 0)
    printf "Average Node Fill Ratio: %.3f\n", (count_nrx ? node_ratio / count_nrx : 0)
}
