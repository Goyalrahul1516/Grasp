#!/usr/bin/env python3
import urllib.request
import csv
import datetime
from time import sleep
import sys

READ_API_KEY='7IUHQD5JM0Q36LTU'
CHANNEL_ID=2588719

start_time=datetime.datetime.strptime('2014-04-16 07:25:08 UTC','%Y-%m-%d %H:%M:%S %Z')
results=100 # fetch this many results (max 8000)
output_file = 'thingspeak_data.csv'

def main():
    end_time=datetime.datetime.utcnow() # or set to desired value like above
    err_count=0
    
    with open(output_file, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(['Timestamp', 'Field1', 'Field2', 'Field3', 'Field4', 'Field5', 'Field6', 'Field7', 'Field8'])  # Adjust field names if needed

        while end_time > start_time:
            end_timestr = urllib.parse.quote(datetime.datetime.strftime(end_time,'%Y-%m-%d %H:%M:%S %Z'))
            url = "https://api.thingspeak.com/channels/%d/feeds.csv?results=%d&api_key=%s&end=%s" \
                  % (CHANNEL_ID, results, READ_API_KEY, end_timestr)
            try:
                conn = urllib.request.urlopen(url)
            except urllib.error.HTTPError as e:
                sys.stderr.write(f"HTTP error: {e.code}")
                err_count += 1
                if err_count > 5:
                    sys.stderr.write("Too many errors, giving up")
                    break
                sleep(10)
                continue
            except urllib.error.URLError as e:
                sys.stderr.write(f"URL error: {e.reason}")
                err_count += 1
                if err_count > 5:
                    sys.stderr.write("Too many errors, giving up")
                    break
                sleep(10)
                continue

            err_count = 0
            response = conn.read().decode('utf-8')
            data = response.split('\n')
            # print in reverse order
            data2 = sorted(filter((lambda x: x != ''), data[1:]), reverse=True)
            if len(data2) <= 0:
                sys.stderr.write("No more data")
                break
            for d in data2:
                csvwriter.writerow(d.split(','))  # Write each line to CSV
            # get the last (earliest) timestamp
            dtime = data2[-1].split(',')[0]
            # subtract 1 second to avoid duplicates
            end_time = datetime.datetime.strptime(dtime, '%Y-%m-%d %H:%M:%S %Z') - datetime.timedelta(seconds=1)
            conn.close()
            sleep(5)

    sys.stderr.write("\nFinished")


if __name__ == '__main__':
    main()
