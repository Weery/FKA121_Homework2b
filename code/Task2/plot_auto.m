close all, clear all, clc

data = importdata('auto_correlation.dat');

plot(data(:,1), data(:,2))
hold on
plot(data(11,1),data(11,2),'r*')

xlabel('VET INTE VAD SOM SKA ST� H�R JUST NU','interpreter','latex','fontsize',19)
ylabel('VET INTE VAD SOM SKA ST� H�R JUST NU','interpreter','latex','fontsize',19)
title('VET INTE VAD SOM SKA ST� H�R JUST NU, men n�got med decay och s och typ exp(-2)','interpreter','latex','fontsize',19)
