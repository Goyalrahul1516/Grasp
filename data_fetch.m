clear;
clc;
ChannelID = 2588719;
datafieldID = 1:6;
readAPIKey = '7IUHQD5JM0Q36LTU';

[data,timeStamp1] = thingSpeakRead(ChannelID,'Fields',datafieldID,'NumPoints',100,'ReadKey',readAPIKey,OutputFormat='TimeTable')
