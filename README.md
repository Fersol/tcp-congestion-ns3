# tcp-congestion-ns3
Research tcp congestion algorithm in ns3

To run script download ns3

run

python build.py

./waf configure

After this place tcp-congestion.cc into scratch directory 

To run this script use

./waf --run tcp-congestion

It will create trace "cwnd.tr" for cwnd changing

To plot this graphic use plot.py. It will create "cwnd.png" file.



