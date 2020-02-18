% Ingrid Lan - Feb. 2020
% Hughes Section 9.3 Exercise 16
% Linear IVP: Md'' + Kd = 0, where d = [d1; d2]


% System parameters
m1    = 1.0;
m2    = 1.0;
k1    = 1.0E4;
k2    = 1.0;

M = [m1, 0; 0, m2];
K = [k1 + k2, -k2; -k2, k2];

% Determine eigenvalues & eigenvectors
% Substituting d = A*sin(wt) into the system yields 
% -M A w^2 sin(wt) + K A sin(wt) = 0
% K A = w^2 M A
[evec, eval] = eig(K, M);
w = sqrt(diag(eval));

% Time stepping
T  = 25;
dt = T / 100;
t = 0 : dt : T;

% (a): damped_Newmark
% (b): gen-alpha-2nd_order_IVP
% (c): gen-alpha-1st_order_IVP
method = 'b';

if strcmp(method, 'a')
    % Damped Newmark-beta parameters
    beta  = 0.3025;
    gamma = 0.6;
else
    % Generalized-alpha parameters
    rho_inf = 0.5;
    alpha_m = 0.5 * (3 - rho_inf) / (1 + rho_inf);
    alpha_f = 1 / (1 + rho_inf);
    gamma   = 0.5 + alpha_m - alpha_f;
    beta    = (1 + alpha_m - alpha_f)^2 / 4;       % only for 2nd-order IVP
end

switch method
    
    % ================= Damped Newmark-beta ================= %
    case 'a'
        
        method_name = 'damped_Newmark';

        d = zeros(2, length(t));        % Displacement
        v = zeros(2, length(t));        % Velocity
        a = zeros(2, length(t));        % Acceleration
        r = zeros(2, length(t));        % Residual (for debugging)
        
        % Initial conditions
        d(:, 1) = [1; 10];
        v(:, 1) = [0;  0];
        a(:, 1) = M \ (-K * d(:, 1));

        
        for i = 2 : length(t)
            
            % Predictors
            d_predictor = d(:, i - 1) + dt * v(:, i - 1) + ...
                          dt^2 / 2 * (1 - 2 * beta) * a(:, i - 1);
            v_predictor = v(:, i - 1) + (1 - gamma) * dt * a(:, i - 1);
            
            % Recursion relation for a_(n+1)
            a(:, i) = (M + beta * dt^2 * K) \ -K * d_predictor;
            
            d(:, i) = d_predictor + beta * dt^2 * a(:, i);
            v(:, i) = v_predictor + gamma * dt * a(:, i);
            
            % Compute residual
            r(:, i) = M * a(:, i) + K * d(:, i);
            
        end
        
        
    % ================= Gen-alpha for 2nd-order IVP ================= %
    case 'b'
        
        method_name = 'gen-alpha-2nd_order_IVP';
        
        % Reference: Chung & Hulbert (1993). A time integration algorithm
        % for structural dynamics with improved numerical dissipation: The
        % generalized-alpha method.
        
        d = zeros(2, length(t));        % Displacement
        v = zeros(2, length(t));        % Velocity
        a = zeros(2, length(t));        % Acceleration
        r = zeros(2, length(t));        % Residual (for debugging)
        
        % Initial conditions
        d(:, 1) = [1; 10];
        v(:, 1) = [0;  0];
        a(:, 1) = M \ (-K * d(:, 1));
        
        for i = 2 : length(t)
            d_n = d(:, i - 1);
            v_n = v(:, i - 1);
            a_n = a(:, i - 1);
            
            % Solve K_eff * a_(n+1) = F_eff
            K_eff = alpha_m * M + alpha_f * beta * dt^2 * K;
            F_eff = -(1 - alpha_m) * M * a_n - ...
                    K * ( d_n + alpha_f * dt * v_n + ...
                    alpha_f * dt^2 / 2 * (1 - 2 * beta) * a_n );
                
            a(:, i) = K_eff \ F_eff;
            v(:, i) = v_n + dt * ( (1 - gamma) * a_n + gamma * a(:, i) );
            d(:, i) = d_n + dt * v_n + dt^2 / 2 * ...
                      ( (1 - 2 * beta) * a_n + 2 * beta * a(:, i) );
            
            % Compute residual
            r(:, i) = M * a(:, i) + K * d(:, i);
            
        end
        
        
        
    % ================= Gen-alpha for 1st-order IVP ================= %
    case 'c'
        
        method_name = 'gen-alpha-1st_order_IVP';
        
        % Reference: Kadapa et al. (2017). On the advantages of using the
        % first-order generalized-alpha scheme for structural dynamic problems.
        
        d = zeros(2, length(t));        % Displacement
        v = zeros(2, length(t));        % Velocity
        dot_d = zeros(2, length(t));    % Time derivative of displacement
        dot_v = zeros(2, length(t));    % Time derivative of velocity
        r = zeros(2, length(t));        % Residual (for debugging)
        
        % Initial conditions
        d(:, 1) = [1; 10];
        v(:, 1) = [0;  0];
        dot_d(:, 1) = [0; 0];
        dot_v(:, 1) = M \ (-K * d(:, 1));
        
        for i = 2 : length(t)
            
            d_n = d(:, i - 1);
            v_n = v(:, i - 1);
            dot_d_n = dot_d(:, i - 1);
            dot_v_n = dot_v(:, i - 1);
            
            % Solve K_eff * d_(n+1) = F_eff
            K_eff = alpha_m^2 / (alpha_f * gamma^2 * dt^2) * M + alpha_f * K;
            F_eff = -(1 - alpha_m) * M * dot_v_n - (1 - alpha_f) * K * d_n + ...
                    alpha_m * M * ( alpha_m / (alpha_f * gamma^2 * dt^2) * d_n + ...
                    1 / (alpha_f * gamma * dt) * v_n - (gamma - 1) / gamma * dot_v_n - ...
                    (gamma - alpha_m) / (alpha_f * gamma^2 * dt) * dot_d_n );
            
            d(:, i) = K_eff \ F_eff;
            dot_d(:, i) = 1 / (gamma * dt) * ( d(:, i) - d_n ) + ...
                          (gamma - 1) / gamma * dot_d_n;
            v(:, i) = alpha_m / (alpha_f * gamma * dt) * ( d(:, i) - d_n ) + ...
                      (gamma - alpha_m) / (gamma * alpha_f) * dot_d_n + ...
                      (alpha_f - 1) / alpha_f * v_n;
            dot_v(:, i) = alpha_m / (alpha_f * gamma^2 * dt^2) * ( d(:, i) - d_n ) - ...
                          1 / (alpha_f * gamma * dt) * v_n + ...
                          (gamma - 1) / gamma * dot_v_n + ...
                          (gamma - alpha_m) / (alpha_f * gamma^2 * dt) * dot_d_n;
            
            % Compute residual
            r(:, i) = M * dot_v(:, i) + K * d(:, i);
            
        end
        

end


figure;
subplot(3, 1, 1); plot(0 : (length(t) - 1), d(1, :)); 
ylabel('d1'); xlim([0, 100]); ylim([-2, 2])
subplot(3, 1, 2); plot(0 : (length(t) - 1), v(1, :)); 
ylabel('v1'); xlim([0, 100]); % ylim([-80, 80]);
subplot(3, 1, 3); plot(1 : length(t), r(1, :));
ylabel('Residual'); xlim([0, 100]); xlabel('n');
%saveas(gcf, [method_name, '_1.png']);

figure;
subplot(3, 1, 1); plot(0 : (length(t) - 1), d(2, :));
ylabel('d2'); xlim([0, 100]); ylim([-20, 20]);
subplot(3, 1, 2); plot(0 : (length(t) - 1), v(2, :));
ylabel('v2'); xlim([0, 100]); ylim([-20, 20]);
subplot(3, 1, 3); plot(1 : length(t), r(2, :)); xlabel('n');
ylabel('Residual'); xlim([0, 100]);

%saveas(gcf, [method_name, '_2.png']);