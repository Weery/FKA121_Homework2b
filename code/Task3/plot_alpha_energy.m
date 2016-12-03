clear all, close all, clc

alpha_energy=importdata('alpha_avg_energy.dat');

mean_alpha = mean(alpha_energy(:,2:end),2);
var_alpha = var(alpha_energy(:,2:end),0,2);

plot(alpha_energy(:,1),mean_alpha)

% not necessarily right
errorbar(alpha_energy(:,1),mean_alpha,var_alpha*100)

hold on
[min_alpha, min_alpha_idx]=min(mean_alpha);
plot(alpha_energy(min_alpha_idx,1),mean_alpha(min_alpha_idx),'rx')

min_energy_plot=-2.879;

plot([alpha_energy(min_alpha_idx,1) alpha_energy(min_alpha_idx,1)],[min_energy_plot,mean_alpha(min_alpha_idx)],'k--')
plot([alpha_energy(1,1) alpha_energy(min_alpha_idx,1)],[mean_alpha(min_alpha_idx),mean_alpha(min_alpha_idx)],'k--')

alpha_txt = sprintf('%f', alpha_energy(min_alpha_idx,1));
alpha_txt = strcat('$\leftarrow \alpha = ',alpha_txt, '$');

text(alpha_energy(min_alpha_idx,1)+0.001,mean_alpha(min_alpha_idx)+(min_energy_plot-mean_alpha(min_alpha_idx))/2,alpha_txt,'interpreter','latex')

energy_txt = sprintf('%f', mean_alpha(min_alpha_idx));
energy_txt = strcat('$\uparrow E = ',energy_txt, '$');

text((alpha_energy(min_alpha_idx,1)+alpha_energy(1,1))/2-0.03,mean_alpha(min_alpha_idx)-0.0002,energy_txt,'interpreter','latex')

xlim([alpha_energy(1,1),alpha_energy(end,1)])

ylabel('Energy [$a.u.$]','interpreter','latex')
xlabel('$\alpha$ [$\#$]','interpreter','latex')
title('Estimation of lowest energy based on independent $\alpha$ runs','interpreter','latex')

