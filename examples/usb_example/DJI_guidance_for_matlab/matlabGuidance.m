function matlabGuidance
% demo_Guidance
DISPLAY_IMAGE = 1;
fig_img = 1;
fig_vo = 2;
fig_imu = 3;
%% Read image data
filename_imleft = 'imleft.dat';
% Create the communications file if it is not already there.
while ~exist(filename_imleft, 'file')    
    fprintf('Cannot open file "%s"', filename_imleft);
    pause(1);
end
% Memory map the file.
m_imleft = memmapfile(filename_imleft, 'Writable', true, 'Format', {'uint8',[320,240],'im';'int32',[1,1],'hasData'});

%% Read depth data
filename_depth = 'imdepth.dat';
% Create the communications file if it is not already there.
while ~exist(filename_depth, 'file')    
    fprintf('Cannot open file "%s"', filename_depth);
    pause(1);
end
% Memory map the file.
m_depth = memmapfile(filename_depth, 'Writable', true, 'Format', {'uint16',[320,240],'im';'int32',[1,1],'hasData'});

%% Read IMU data
filename_imu = 'imu.dat';
% Create the communications file if it is not already there.
while ~exist(filename_imu, 'file')    
    fprintf('Cannot open file "%s"', filename_imu);
    pause(1);
end
% Memory map the file.
m_imu = memmapfile(filename_imu, 'Writable', true, 'Format', {'int32',[1,1],'hasData';'single',[1,3],'acc';'single',[1,4],'q'});

%% Read VO data
filename_vo = 'vo.dat';
% Create the communications file if it is not already there.
while ~exist(filename_vo, 'file')    
    fprintf('Cannot open file "%s"', filename_vo);
    pause(1);
end
% Memory map the file.
m_vo = memmapfile(filename_vo, 'Writable', true, 'Format', {'int32',[1,1],'hasData';'int16',[1,3],'vel'});
fileInfo = dir(filename_vo);
vo_available = 1;
if(fileInfo.bytes<10)
    vo_available = 0;
end
vel_all = zeros(100,3);
%% the main loop to extract data from file
while true
    % Wait until the first byte is not zero.
    if m_imleft.Data.hasData
        imleft = m_imleft.Data.im';  % transpose to convert C array to matlab array
        if DISPLAY_IMAGE
        figure(fig_img); subplot(121); imshow(imleft);
        end
        m_imleft.Data.hasData = int32(0);
    end
    
    if m_depth.Data.hasData
        imdepth = m_depth.Data.im';
        if DISPLAY_IMAGE
        subplot(122); imshow(uint8(imdepth));
        end
        m_depth.Data.hasData = int32(0);
    end
    
    if m_imu.Data.hasData
        acc = m_imu.Data.acc;
        q = m_imu.Data.q;
        disp([acc,q]);

        m_imu.Data.hasData = int32(0);
    end
    
    if vo_available
    if m_vo.Data.hasData
        vel = m_vo.Data.vel;        
        disp(vel);
        
        vel_all = [vel_all(2:end,:); vel];
        figure(fig_vo); 
        subplot(131); plot(vel_all(:,1)); title('x');
        subplot(132); plot(vel_all(:,2)); title('y');
        subplot(133); plot(vel_all(:,3)); title('z');
        
        m_vo.Data.hasData = int32(0);
    end
    end
    
    pause(.01);
end


