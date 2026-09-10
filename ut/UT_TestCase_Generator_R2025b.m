function UT_TestCase_Generator_R2025b()
%==========================================================================
% UT_TestCase_Generator_R2025b.m
%
% MATLAB R2025b
%
% 功能：
%   1) 弹框输入“输入信号个数”
%   2) 弹框输入“时间周期 Ts”和“结束时间 EndTime”
%   3) 根据输入信号个数动态创建信号配置表
%   4) 支持常用 Simulink/MATLAB 数据类型：
%        double, single, half
%        boolean
%        int8/uint8/int16/uint16/int32/uint32/int64/uint64
%        Fixed Point, Enumerated, Bus, ValueType, string
%   5) 从 t=0 开始，每隔 Ts 生成一个时间点，直到 <= EndTime
%   6) 每个时间点随机生成一组输入数据
%   7) 生成 Excel：
%        TestCases  : 时间轴 + 所有输入信号
%        DataConfig : 信号名称/类型/范围
%        TimeConfig : Ts/EndTime/测试点数
%        README     : 使用说明
%        ImportExample : MATLAB导入示例
%
% 注意：
%   Excel 本身不保存 Simulink 的完整类型语义，因此 DataConfig Sheet
%   用于保存原始类型信息。UT程序导入 Excel 后，应依据 DataConfig 进行
%   类型转换/构造。
%
%==========================================================================

clc;
fprintf('\n============================================================\n');
fprintf('       MATLAB / Simulink UT Test Case Generator\n');
fprintf('                    MATLAB R2025b\n');
fprintf('============================================================\n\n');

%% ========================================================================
% 1. 输入信号个数
% =========================================================================
answer = inputdlg( ...
    {'请输入输入信号个数：'}, ...
    'UT输入信号数量', ...
    [1 45], ...
    {'5'});

if isempty(answer)
    fprintf('用户取消操作。\n');
    return;
end

signalNum = str2double(strtrim(answer{1}));

if ~isfinite(signalNum) || signalNum < 1 || signalNum ~= floor(signalNum)
    errordlg('输入信号个数必须是大于0的整数！', '输入错误');
    return;
end

%% ========================================================================
% 2. 输入时间周期和结束时间
% =========================================================================
answer = inputdlg( ...
    {'请输入测试时间周期 Ts（单位：s）：', ...
     '请输入测试结束时间 EndTime（单位：s）：'}, ...
    'UT时间配置', ...
    [1 50; 1 50], ...
    {'0.01','1'});

if isempty(answer)
    fprintf('用户取消操作。\n');
    return;
end

sampleTime = str2double(strtrim(answer{1}));
endTime    = str2double(strtrim(answer{2}));

if ~isfinite(sampleTime) || sampleTime <= 0
    errordlg('Ts 必须是大于0的数值！', '输入错误');
    return;
end

if ~isfinite(endTime) || endTime < 0
    errordlg('EndTime 必须是大于等于0的数值！', '输入错误');
    return;
end

% 使用整数索引生成时间，避免反复 t=t+Ts 带来的浮点累计误差
testCaseNum = floor(endTime / sampleTime + 1e-12) + 1;
timeIndex = (0:testCaseNum-1)';
Time_s = timeIndex .* sampleTime;

% 避免 0.9999999999999999 / 1.0000000000000002 之类的显示问题
digits = max(6, min(15, ceil(-log10(sampleTime)) + 6));
Time_s = round(Time_s, digits);

% 最后一个点严格不能超过 EndTime
valid = Time_s <= endTime + max(1e-12, abs(endTime)*1e-12);
Time_s = Time_s(valid);
testCaseNum = numel(Time_s);

fprintf('输入信号数量：%d\n', signalNum);
fprintf('测试周期：%.15g s\n', sampleTime);
fprintf('结束时间：%.15g s\n', endTime);
fprintf('自动生成测试点数：%d\n\n', testCaseNum);

%% ========================================================================
% 3. 支持的数据类型
% =========================================================================
dataTypes = { ...
    'double', ...
    'single', ...
    'half', ...
    'boolean', ...
    'int8', ...
    'uint8', ...
    'int16', ...
    'uint16', ...
    'int32', ...
    'uint32', ...
    'int64', ...
    'uint64', ...
    'string', ...
    'Fixed Point', ...
    'Enumerated', ...
    'Bus', ...
    'ValueType'};

%% ========================================================================
% 4. 默认信号配置
% =========================================================================
tableData = cell(signalNum,5);

defaultNames = {'signal1','signal2','signal3','signal4','signal5','signal6','signal7','signal8','signal9','signal10'};

defaultTypes = {'uint8','uint16','int16','single','double', ...
                'boolean','double','double','double','single'};

defaultMins = {'0','0','0','0','0','0','0','0','0','0'};
defaultMaxs = {'0','0','0','0','0','0','0','0','0','0'};

for i = 1:signalNum
    if i <= numel(defaultNames)
        tableData{i,1} = defaultNames{i};
    else
        tableData{i,1} = sprintf('Signal%d',i);
    end

    if i <= numel(defaultTypes)
        tableData{i,2} = defaultTypes{i};
    else
        tableData{i,2} = 'double';
    end

    tableData{i,3} = '';

    if i <= numel(defaultMins)
        tableData{i,4} = defaultMins{i};
        tableData{i,5} = defaultMaxs{i};
    else
        tableData{i,4} = '0';
        tableData{i,5} = '1';
    end
end

%% ========================================================================
% 5. 创建GUI
% =========================================================================
fig = uifigure( ...
    'Name','UT输入信号配置 - MATLAB R2025b', ...
    'Position',[80 50 1250 760], ...
    'Resize','on');

uilabel(fig, ...
    'Text','UT输入信号配置', ...
    'FontSize',22, ...
    'FontWeight','bold', ...
    'HorizontalAlignment','center', ...
    'Position',[450 705 350 35]);

timeInfo = sprintf( ...
    '输入信号：%d 个    |    Ts：%.12g s    |    EndTime：%.12g s    |    自动测试点：%d', ...
    signalNum, sampleTime, endTime, testCaseNum);

uilabel(fig, ...
    'Text',timeInfo, ...
    'FontSize',12, ...
    'FontWeight','bold', ...
    'Position',[45 665 1160 25]);

uilabel(fig, ...
    'Text',['说明：双击/直接编辑表格单元格。Data Type 使用下拉框。' ...
            'Fixed Point / Enum / Bus / ValueType 在 Type Definition 中填写定义。'], ...
    'FontSize',11, ...
    'Position',[45 635 1160 25]);

%% ========================================================================
% 6. 输入信号配置表
% =========================================================================
tbl = uitable(fig, ...
    'Data',tableData, ...
    'ColumnName',{'Signal Name','Data Type','Type Definition','Min Value','Max Value'}, ...
    'ColumnEditable',[true true true true true], ...
    'ColumnFormat',{'char',dataTypes,'char','char','char'}, ...
    'RowName',[], ...
    'Position',[40 300 1170 315], ...
    'Tag','SignalTable');

tbl.ColumnWidth = {210,180,310,150,150};

%% ========================================================================
% 7. 使用说明
% =========================================================================
instructionText = { ...
    '使用说明：'; ...
    ''; ...
    '1. Signal Name：输入 UT 输入变量名称，例如 Vin、Vout、Iout、Enable。'; ...
    '2. Data Type：选择数据类型。'; ...
    '3. 普通数值类型：填写 Min / Max，随机值将在范围内生成。'; ...
    '4. boolean：自动生成 false/true（Excel中表现为0/1）。'; ...
    '5. string：Min/Max忽略，自动生成 Test_00001、Test_00002...。'; ...
    '6. Fixed Point：Type Definition 示例：fixdt(1,16,8)。'; ...
    '7. Enumerated：Type Definition 填写已经存在于 MATLAB 工作区的 Enum Class 名称。'; ...
    '8. Bus：Type Definition 填写已经存在于 MATLAB base workspace 的 Simulink.Bus 对象名称。'; ...
    '9. ValueType：Type Definition 填写已经存在于 MATLAB base workspace 的 Simulink.ValueType 对象名称。'; ...
    '10. 时间轴从0秒开始，每隔Ts生成一个测试点，直到不超过EndTime。'; ...
    '11. Excel 的 DataConfig Sheet 保存原始数据类型，TestCases Sheet 用于实际测试数据读取。' ...
    };

uilabel(fig, ...
    'Text',instructionText, ...
    'FontSize',10.5, ...
    'Position',[50 55 830 225], ...
    'VerticalAlignment','top');

%% ========================================================================
% 8. 生成按钮
% =========================================================================
uibutton(fig, ...
    'Text','确认并生成测试用例 Excel', ...
    'FontSize',14, ...
    'FontWeight','bold', ...
    'Position',[900 105 270 60], ...
    'ButtonPushedFcn',@(btn,event)generateExcel( ...
        fig,sampleTime,endTime,Time_s));

end


%% ########################################################################
% 生成Excel
% ########################################################################
function generateExcel(fig,sampleTime,endTime,Time_s)

tbl = findobj(fig,'Tag','SignalTable');

if isempty(tbl)
    uialert(fig,'找不到输入信号配置表。','内部错误');
    return;
end

tableData = tbl.Data;
signalNum = size(tableData,1);
testCaseNum = numel(Time_s);

%% ========================================================================
% 读取配置
% =========================================================================
signalNames = strings(signalNum,1);
dataTypes = strings(signalNum,1);
typeDefinitions = strings(signalNum,1);
minValues = nan(signalNum,1);
maxValues = nan(signalNum,1);

for i = 1:signalNum

    signalNames(i) = strtrim(string(tableData{i,1}));
    dataTypes(i) = strtrim(string(tableData{i,2}));
    typeDefinitions(i) = strtrim(string(tableData{i,3}));

    if signalNames(i) == ""
        uialert(fig,sprintf('第 %d 个信号名称不能为空！',i),'输入错误');
        return;
    end

    if dataTypes(i) == ""
        uialert(fig,sprintf('第 %d 个信号的数据类型不能为空！',i),'输入错误');
        return;
    end

    if isSpecialType(dataTypes(i))
        minValues(i) = NaN;
        maxValues(i) = NaN;
    else
        minText = strtrim(string(tableData{i,4}));
        maxText = strtrim(string(tableData{i,5}));

        minValues(i) = str2double(minText);
        maxValues(i) = str2double(maxText);

        if isnan(minValues(i)) || isnan(maxValues(i))
            uialert(fig, ...
                sprintf('%s 的 Min Value / Max Value 必须是有效数字！',signalNames(i)), ...
                '输入错误');
            return;
        end

        if minValues(i) > maxValues(i)
            uialert(fig, ...
                sprintf('%s 的 Min Value 不能大于 Max Value！',signalNames(i)), ...
                '输入错误');
            return;
        end
    end
end

% 关键：强制转 string，避免 R2025b 下 unique(cell) 相关错误
signalNames = string(signalNames);

%% ========================================================================
% 检查重复名称
% =========================================================================
if numel(unique(signalNames)) ~= signalNum
    uialert(fig,'Signal Name 不能重复！','输入错误');
    return;
end

%% ========================================================================
% 转成合法 MATLAB 变量名
% =========================================================================
validNames = matlab.lang.makeValidName(signalNames);

if numel(unique(validNames)) ~= signalNum
    uialert(fig, ...
        ['信号名称转换为 MATLAB 变量名后发生重复。' newline ...
         '例如 A-B 和 A_B 会发生冲突，请重新命名。'], ...
        '信号名称错误');
    return;
end

%% ========================================================================
% 检查高级数据类型配置
% =========================================================================
for i = 1:signalNum

    type = dataTypes(i);
    typeDef = typeDefinitions(i);

    if type == "Fixed Point" || ...
       type == "Enumerated" || ...
       type == "Bus" || ...
       type == "ValueType"

        if typeDef == ""
            uialert(fig, ...
                sprintf('%s 的 Type Definition 不能为空！',signalNames(i)), ...
                '输入错误');
            return;
        end
    end

    if type == "boolean"
        % Boolean不强制要求Min/Max，实际生成固定为0/1
        continue;
    end
end

%% ========================================================================
% 初始化结果表
% =========================================================================
TestCase_ID = (1:testCaseNum)';
resultTable = table(TestCase_ID,Time_s(:),'VariableNames',{'TestCase_ID','Time_s'});

rng('shuffle');

%% ========================================================================
% 逐信号生成测试数据
% =========================================================================
for i = 1:signalNum

    type = dataTypes(i);
    typeDef = typeDefinitions(i);
    minVal = minValues(i);
    maxVal = maxValues(i);
    colName = char(validNames(i));

    try

        switch type

            case "double"
                values = minVal + (maxVal-minVal).*rand(testCaseNum,1);

            case "single"
                values = single(minVal + (maxVal-minVal).*rand(testCaseNum,1));

            case "half"
                raw = minVal + (maxVal-minVal).*rand(testCaseNum,1);
                values = half(raw);
                % Excel/UT读取层通常使用double承载half数值
                values = double(values);

            case "boolean"
                values = logical(randi([0 1],testCaseNum,1));

            case "string"
                values = strings(testCaseNum,1);
                for k = 1:testCaseNum
                    values(k) = sprintf('Test_%05d',k);
                end

            case "int8"
                values = generateIntegerData('int8',minVal,maxVal,testCaseNum);

            case "uint8"
                values = generateIntegerData('uint8',minVal,maxVal,testCaseNum);

            case "int16"
                values = generateIntegerData('int16',minVal,maxVal,testCaseNum);

            case "uint16"
                values = generateIntegerData('uint16',minVal,maxVal,testCaseNum);

            case "int32"
                values = generateIntegerData('int32',minVal,maxVal,testCaseNum);

            case "uint32"
                values = generateIntegerData('uint32',minVal,maxVal,testCaseNum);

            case "int64"
                values = generateIntegerData('int64',minVal,maxVal,testCaseNum);

            case "uint64"
                values = generateIntegerData('uint64',minVal,maxVal,testCaseNum);

            case "Fixed Point"
                values = generateFixedPointData(typeDef,minVal,maxVal,testCaseNum);
                % Excel保存Real World Value
                values = double(values);

            case "Enumerated"
                values = generateEnumData(typeDef,testCaseNum);
                % Excel保存Enum underlying value
                values = double(values);

            case "Bus"
                resultTable = expandBusToTable( ...
                    resultTable,typeDef,testCaseNum,validNames(i));
                continue;

            case "ValueType"
                values = generateValueTypeData( ...
                    typeDef,minVal,maxVal,testCaseNum);

            otherwise
                error('不支持的数据类型：%s',type);

        end

        resultTable.(colName) = values;

    catch ME
        uialert(fig, ...
            sprintf(['信号 "%s" 生成失败。\n\n' ...
                     'Data Type：%s\n' ...
                     'Type Definition：%s\n\n' ...
                     '%s'], ...
                     signalNames(i),type,typeDef,ME.message), ...
            '数据生成错误');
        return;
    end
end

%% ========================================================================
% 保存Excel
% =========================================================================
timeString = datestr(now,'yyyymmdd_HHMMSS');

defaultFileName = sprintf( ...
    'UT_TestCases_Ts_%g_End_%g_%s.xlsx', ...
    sampleTime,endTime,timeString);

[fileName,pathName] = uiputfile( ...
    '*.xlsx', ...
    '保存UT测试用例Excel文件', ...
    defaultFileName);

if isequal(fileName,0)
    fprintf('用户取消保存。\n');
    return;
end

fullFileName = fullfile(pathName,fileName);

%% ========================================================================
% TestCases
% =========================================================================
writetable(resultTable,fullFileName,'Sheet','TestCases');

%% ========================================================================
% DataConfig
% =========================================================================
DataConfig = table( ...
    signalNames, ...
    dataTypes, ...
    typeDefinitions, ...
    minValues, ...
    maxValues, ...
    validNames, ...
    'VariableNames', ...
    {'SignalName','DataType','TypeDefinition','MinValue','MaxValue','MATLABName'});

writetable(DataConfig,fullFileName,'Sheet','DataConfig');

%% ========================================================================
% TimeConfig
% =========================================================================
TimeConfig = table( ...
    sampleTime, ...
    endTime, ...
    testCaseNum, ...
    'VariableNames', ...
    {'SampleTime_s','EndTime_s','TestCaseCount'});

writetable(TimeConfig,fullFileName,'Sheet','TimeConfig');

%% ========================================================================
% README
% =========================================================================
Description = { ...
    'MATLAB / Simulink UT Test Case File'; ...
    ''; ...
    'TestCases：时间轴 + 每个时间点的UT输入数据。'; ...
    'Config：保存原始信号名称、数据类型、类型定义、Min/Max及MATLAB合法变量名。'; ...
    'TimeConfig：保存Ts、EndTime、测试点数。'; ...
    'README：本文件说明。'; ...
    'ImportExample：MATLAB读取Excel示例。'; ...
    ''; ...
    '时间规则：t=0开始，每隔Ts产生一个测试点，直到不超过EndTime。'; ...
    '测试点数量 = floor(EndTime/Ts) + 1（最后一点不超过EndTime）。'; ...
    ''; ...
    'Boolean：Excel中使用0/1表示。'; ...
    'Fixed Point：Excel中保存Real World Value。'; ...
    'Enumerated：Excel中保存Underlying Value。'; ...
    'Bus：如果Bus Object存在，则按Bus Element展开成多列。'; ...
    'string：生成Test_00001、Test_00002...。'; ...
    ''; ...
    '注意：Excel不是Simulink类型容器，真正UT执行时请依据Config Sheet恢复目标数据类型。' ...
    };

writecell(Description,fullFileName,'Sheet','README');

%% ========================================================================
% ImportExample
% =========================================================================
exampleCode = { ...
    '% MATLAB读取UT测试用例'; ...
    sprintf('fileName = ''%s'';',strrep(fullFileName,'''','''''')); ...
    'T = readtable(fileName,''Sheet'',''TestCases'');'; ...
    'DataConfig = readtable(fileName,''Sheet'',''DataConfig'');'; ...
    ''; ...
    '% 时间轴'; ...
    't = T.Time_s;'; ...
    ''; ...
    '% 示例：读取某个输入信号'; ...
    '% x = T.x;'; ...
    '% y = T.y;'; ...
    ''; ...
    '% 根据Config恢复目标数据类型的示例：'; ...
    '% idx = strcmp(DataConfig.SignalName,''x'');'; ...
    '% targetType = string(DataConfig.DataType(idx));'; ...
    '% x = cast(T.x,targetType);'; ...
    ''; ...
    '% 在MATLAB Unit Test中，可以按行读取T作为每一个时间点输入。' ...
    };

writecell(exampleCode,fullFileName,'Sheet','ImportExample');

%% ========================================================================
% 完成
% =========================================================================
close(fig);

fprintf('============================================================');
fprintf('UT测试用例生成完成！');
fprintf('Excel：%s',fullFileName);
fprintf('输入信号：%d 个',signalNum);
fprintf('Sample Time：%.15g s',sampleTime);
fprintf('End Time：%.15g s',endTime);
fprintf('Test Cases：%d',testCaseNum);
fprintf('============================================================');

msgbox( ...
    sprintf(['UT测试用例生成完成！' ...
             '输入信号：%d 个' ...
             'Ts：%.12g s' ...
             'EndTime：%.12g s' ...
             '测试点：%d' ... 
             '文件：%s'], ...
    signalNum,sampleTime,endTime,testCaseNum,fullFileName), ...
'生成完成');

end


%% ########################################################################
% 判断特殊类型
% ########################################################################
function result = isSpecialType(type)

result = ismember(string(type), ...
    ["string","boolean","Fixed Point","Enumerated","Bus","ValueType"]);

end


%% ########################################################################
% 整数随机数据
% ########################################################################
function values = generateIntegerData(type,minVal,maxVal,N)

low = ceil(minVal);
high = floor(maxVal);

% 限制到对应整数类型合法范围
switch type
    case 'int8'
        typeLow = double(intmin('int8'));
        typeHigh = double(intmax('int8'));
    case 'uint8'
        typeLow = double(intmin('uint8'));
        typeHigh = double(intmax('uint8'));
    case 'int16'
        typeLow = double(intmin('int16'));
        typeHigh = double(intmax('int16'));
    case 'uint16'
        typeLow = double(intmin('uint16'));
        typeHigh = double(intmax('uint16'));
    case 'int32'
        typeLow = double(intmin('int32'));
        typeHigh = double(intmax('int32'));
    case 'uint32'
        typeLow = double(intmin('uint32'));
        typeHigh = double(intmax('uint32'));
    case 'int64'
        typeLow = double(intmin('int64'));
        typeHigh = double(intmax('int64'));
    case 'uint64'
        typeLow = 0;
        % double不能精确表达整个uint64范围，因此这里使用uint64边界单独处理
        typeHigh = 2^64-1;
    otherwise
        error('未知整数类型：%s',type);
end

if type ~= "uint64" && type ~= "int64"
    low = max(low,typeLow);
    high = min(high,typeHigh);
else
    % Excel本身对超大整数精度有限，建议UT输入范围不要超过2^53
    low = max(low,typeLow);
    high = min(high,typeHigh);
end

if low > high
    error('Min/Max范围内没有有效整数。');
end

switch type

    case 'int8'
        values = int8(randi([low high],N,1));

    case 'uint8'
        values = uint8(randi([low high],N,1));

    case 'int16'
        values = int16(randi([low high],N,1));

    case 'uint16'
        values = uint16(randi([low high],N,1));

    case 'int32'
        values = int32(randi([low high],N,1));

    case 'uint32'
        values = uint32(randi([low high],N,1));

    case 'int64'
        % 对常见UT范围安全；对于极大int64范围，建议自行扩展为uint64/整数抽样算法
        if low >= -2^53 && high <= 2^53
            values = int64(floor(low + (high-low+1).*rand(N,1)));
        else
            error('int64范围过大，当前随机生成器要求Min/Max落在约±2^53范围内。');
        end

    case 'uint64'
        if low >= 0 && high <= 2^53
            values = uint64(floor(low + (high-low+1).*rand(N,1)));
        else
            error('uint64随机生成当前要求Min/Max不超过2^53。');

        end

    otherwise
        error('未知整数类型：%s',type);
end

end


%% ########################################################################
% Fixed Point
% ########################################################################
function values = generateFixedPointData(typeDef,minVal,maxVal,N)

try
    numericType = eval(typeDef);
catch ME
    error('无法解析Fixed Point定义：%s%s',typeDef,ME.message);
end

if ~isa(numericType,'Simulink.NumericType')
    error('Type Definition必须返回Simulink.NumericType，例如fixdt(1,16,8)。');
end

if isnan(minVal) || isnan(maxVal)
    error('Fixed Point需要填写有效的Min Value和Max Value。');
end

rawValues = minVal + (maxVal-minVal).*rand(N,1);

values = fi( ...
    rawValues, ...
    numericType.SignednessBool, ...
    numericType.WordLength, ...
    numericType.FractionLength, ...
    'RoundingMethod','Nearest', ...
    'OverflowAction','Saturate');

end


%% ########################################################################
% Enumerated
% ########################################################################
function values = generateEnumData(enumClass,N)

enumClass = strtrim(char(enumClass));

if exist(enumClass,'class') ~= 8
    error('找不到Enum Class：%s。请确认Enum类已经在MATLAB路径中。',enumClass);
end

members = enumeration(enumClass);

if isempty(members)
    error('Enum Class %s没有枚举成员。',enumClass);
end

idx = randi([1 numel(members)],N,1);
values = members(idx);

end


%% ########################################################################
% Bus
% ########################################################################
function resultTable = expandBusToTable(resultTable,busObjectName,N,prefix)

busObjectName = strtrim(char(busObjectName));

try
    busObject = evalin('base',busObjectName);
catch ME
    error('无法在base workspace找到Bus Object "%s"：%s',busObjectName,ME.message);
end

if ~isa(busObject,'Simulink.Bus')
    error('变量 "%s" 不是Simulink.Bus对象。',busObjectName);
end

resultTable = expandBusElements(resultTable,busObject,N,char(prefix));

end


function resultTable = expandBusElements(resultTable,busObject,N,prefix)

elements = busObject.Elements;

for i = 1:numel(elements)

    element = elements(i);
    elementName = char(string(element.Name));
    colName = matlab.lang.makeValidName([prefix '_' elementName]);
    dataType = strtrim(char(string(element.DataType)));

    % ---------------------------------------------------------------
    % Nested Bus
    % ---------------------------------------------------------------
    if startsWith(dataType,'Bus:')
        nestedName = strtrim(extractAfter(string(dataType),'Bus:'));
        nestedBus = evalin('base',char(nestedName));

        if ~isa(nestedBus,'Simulink.Bus')
            error('嵌套Bus "%s"不是有效Simulink.Bus对象。',char(nestedName));
        end

        resultTable = expandBusElements( ...
            resultTable,nestedBus,N,colName);

        continue;
    end

    % ---------------------------------------------------------------
    % Bus Element Min/Max
    % ---------------------------------------------------------------
    minVal = element.Min;
    maxVal = element.Max;

    if isempty(minVal) || ~isnumeric(minVal) || ~isscalar(minVal) || ~isfinite(double(minVal))
        minVal = -100;
    end

    if isempty(maxVal) || ~isnumeric(maxVal) || ~isscalar(maxVal) || ~isfinite(double(maxVal))
        maxVal = 100;
    end

    if double(minVal) > double(maxVal)
        tmp = minVal;
        minVal = maxVal;
        maxVal = tmp;
    end

    % ---------------------------------------------------------------
    % 根据Bus Element数据类型生成
    % ---------------------------------------------------------------
    if strcmpi(dataType,'double')

        values = double(minVal) + ...
            (double(maxVal)-double(minVal)).*rand(N,1);

    elseif strcmpi(dataType,'single')

        values = single(double(minVal) + ...
            (double(maxVal)-double(minVal)).*rand(N,1));

    elseif strcmpi(dataType,'boolean') || strcmpi(dataType,'logical')

        values = logical(randi([0 1],N,1));

    elseif ismember(lower(dataType), ...
            {'int8','uint8','int16','uint16','int32','uint32','int64','uint64'})

        values = generateIntegerData( ...
            dataType,double(minVal),double(maxVal),N);

    elseif strcmpi(dataType,'half')

        values = double(half( ...
            double(minVal) + ...
            (double(maxVal)-double(minVal)).*rand(N,1)));

    else
        % 对无法直接解析的Bus Element，保存double随机值。
        % DataConfig/Bus Object仍保留原始类型信息。
        values = double(minVal) + ...
            (double(maxVal)-double(minVal)).*rand(N,1);
    end

    resultTable.(colName) = values;
end

end


%% ########################################################################
% ValueType
% ########################################################################
function values = generateValueTypeData(typeDef,minVal,maxVal,N)

try
    valueTypeObj = evalin('base',strtrim(char(typeDef)));
catch ME
    error('无法在base workspace找到ValueType "%s"：%s',char(typeDef),ME.message);
end

if ~isa(valueTypeObj,'Simulink.ValueType')
    error('变量 "%s" 不是Simulink.ValueType对象。',char(typeDef));
end

if isnan(minVal) || isnan(maxVal)
    minVal = -100;
    maxVal = 100;
end

values = minVal + (maxVal-minVal).*rand(N,1);

end
