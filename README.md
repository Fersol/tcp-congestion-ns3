# tcp-congestion-ns3
Research tcp congestion algorithm in ns3

To run script download ns3:

python build.py

./waf configure

After this place tcp-congestion.cc into scratch directory 

To run this script use:

./waf --run tcp-congestion

To change congestion algorithm (NewReno, Bic, Vegas) and duration use:

./waf --run "tcp-congestion --congestionAlg=Bic --duration=60"

To plot trace from "cwndNewReno.tr" graphic use

python plot.py -files cwndNewReno.tr

It will create "cwndNewReno.png" file.



