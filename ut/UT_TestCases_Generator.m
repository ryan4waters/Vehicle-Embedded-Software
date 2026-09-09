function UT_TestCase_Generator_R2025b()
%==========================================================================
% UT_TestCase_Generator_R2025b
%
% MATLAB R2025b / Simulink
%
% 功能：
%   1. 输入信号数量
%   2. 输入测试用例数量
%   3. GUI配置：
%        Signal Name
%        Data Type
%        Type Definition
%        Min Value
%        Max Value
%   4. 随机生成UT测试数据
%   5. 输出Excel
%
% 支持：
%   Built-in：
%       double
%       single
%       half
%       int8
%       uint8
%       int16
%       uint16
%       int32
%       uint32
%       int64
%       uint64
%       boolean
%       string
%
%   Fixed-point：
%       fixdt(1,16,8)
%       fixdt(0,16,8)
%       ...
%
%   Enumerated：
%       Enum: MyEnum
%
%   Bus：
%       Bus: MyBus
%
%   ValueType：
%       ValueType: MyValueType
%
% Excel：
%
%   TestCases
%       TestCase_ID
%       Signal1
%       Signal2
%       ...
%
%   Config
%       SignalName
%       DataType
%       TypeDefinition
%       MinValue
%       MaxValue
%
%   README
%
% 注意：
%   1. Excel本身不能保存Simulink真实数据类型。
%   2. Config中保存真实类型信息。
%   3. Enum / Bus / ValueType需要用户工程中已经存在对应定义。
%
%==========================================================================

clc;

fprintf('\n');
fprintf('============================================================\n');
fprintf('        MATLAB / Simulink UT Test Case Generator\n');
fprintf('                    MATLAB R2025b\n');
fprintf('============================================================\n');
fprintf('\n');


%% ========================================================================
% 1. 输入信号数量
% =========================================================================

answer = inputdlg( ...
    {'请输入输入信号数量：'}, ...
    'UT测试用例生成器', ...
    [1 40], ...
    {'5'});

if isempty(answer)
    fprintf('用户取消操作。\n');
    return;
end

signalNum = str2double(strtrim(answer{1}));

if ~isfinite(signalNum) || ...
        signalNum <= 0 || ...
        mod(signalNum,1) ~= 0

    errordlg( ...
        '输入信号数量必须是正整数！', ...
        '输入错误');

    return;
end


%% ========================================================================
% 2. 输入测试用例数量
% =========================================================================

answer = inputdlg( ...
    {'请输入测试用例数量：'}, ...
    '测试用例数量', ...
    [1 40], ...
    {'1000'});

if isempty(answer)
    fprintf('用户取消操作。\n');
    return;
end

testCaseNum = str2double(strtrim(answer{1}));

if ~isfinite(testCaseNum) || ...
        testCaseNum <= 0 || ...
        mod(testCaseNum,1) ~= 0

    errordlg( ...
        '测试用例数量必须是正整数！', ...
        '输入错误');

    return;
end


%% ========================================================================
% 3. 创建GUI
% =========================================================================

fig = uifigure( ...
    'Name','UT输入信号配置 - MATLAB R2025b', ...
    'Position',[80 50 1250 720], ...
    'Resize','on');


%% 标题

uilabel(fig, ...
    'Text','UT输入信号配置', ...
    'FontSize',22, ...
    'FontWeight','bold', ...
    'HorizontalAlignment','center', ...
    'Position',[450 670 350 35]);


%% 提示

uilabel(fig, ...
    'Text', ...
    ['普通类型直接选择；Fixed Point / Enum / Bus / ValueType ' ...
     '请在 Type Definition 中填写定义。'], ...
    'FontSize',12, ...
    'Position',[60 635 1100 25]);


%% ========================================================================
% 4. 数据类型下拉选项
% =========================================================================

dataTypes = { ...
    'double', ...
    'single', ...
    'half', ...
    'int8', ...
    'uint8', ...
    'int16', ...
    'uint16', ...
    'int32', ...
    'uint32', ...
    'int64', ...
    'uint64', ...
    'boolean', ...
    'string', ...
    'Fixed Point', ...
    'Enumerated', ...
    'Bus', ...
    'ValueType'};


%% ========================================================================
% 5. 创建默认数据
% =========================================================================

tableData = cell(signalNum,5);

for i = 1:signalNum

    tableData{i,1} = sprintf('Signal_%d',i);

    switch mod(i-1,8)

        case 0
            tableData{i,2} = 'uint8';
            tableData{i,3} = '';
            tableData{i,4} = '0';
            tableData{i,5} = '255';

        case 1
            tableData{i,2} = 'uint16';
            tableData{i,3} = '';
            tableData{i,4} = '0';
            tableData{i,5} = '1000';

        case 2
            tableData{i,2} = 'int16';
            tableData{i,3} = '';
            tableData{i,4} = '-100';
            tableData{i,5} = '100';

        case 3
            tableData{i,2} = 'single';
            tableData{i,3} = '';
            tableData{i,4} = '-100';
            tableData{i,5} = '100';

        case 4
            tableData{i,2} = 'double';
            tableData{i,3} = '';
            tableData{i,4} = '-1';
            tableData{i,5} = '1';

        case 5
            tableData{i,2} = 'boolean';
            tableData{i,3} = '';
            tableData{i,4} = '0';
            tableData{i,5} = '1';

        case 6
            tableData{i,2} = 'int8';
            tableData{i,3} = '';
            tableData{i,4} = '-128';
            tableData{i,5} = '127';

        case 7
            tableData{i,2} = 'uint32';
            tableData{i,3} = '';
            tableData{i,4} = '0';
            tableData{i,5} = '10000';

    end
end


%% ========================================================================
% 6. 创建表格
% =========================================================================

tbl = uitable(fig, ...
    'Data',tableData, ...
    'ColumnName', ...
    {'Signal Name', ...
     'Data Type', ...
     'Type Definition', ...
     'Min Value', ...
     'Max Value'}, ...
    'ColumnEditable', ...
    [true true true true true], ...
    'ColumnFormat', ...
    {'char',dataTypes,'char','char','char'}, ...
    'RowName',[], ...
    'Position',[40 220 1170 390], ...
    'Tag','SignalTable');


%% 设置列宽

tbl.ColumnWidth = {210,180,280,150,150};


%% ========================================================================
% 7. 说明
% =========================================================================

instructionText = {
    '使用说明：'
    '1. Signal Name：输入信号名称，例如 VehicleSpeed、DcCurrent、Enable。'
    '2. Data Type：选择Simulink数据类型。'
    '3. Type Definition：Fixed Point / Enum / Bus / ValueType填写具体定义。'
    '4. 普通数值类型填写Min / Max。'
    '5. boolean建议Min=0，Max=1。'
    '6. string不需要Min / Max。'
    '7. Enum / Bus / ValueType不直接使用Min / Max。'
    ''
    'Fixed Point 示例：fixdt(1,16,8)'
    'Enum 示例：MyEnum'
    'Bus 示例：MyBus'
    'ValueType 示例：MyValueType'
    };

uilabel(fig, ...
    'Text',instructionText, ...
    'FontSize',11, ...
    'Position',[50 45 1000 160], ...
    'VerticalAlignment','top');


%% ========================================================================
% 8. 生成按钮
% =========================================================================

uibutton(fig, ...
    'Text','确认并生成测试用例 Excel', ...
    'FontSize',14, ...
    'FontWeight','bold', ...
    'Position',[900 90 270 55], ...
    'ButtonPushedFcn', ...
    @(btn,event)generateExcel(fig,testCaseNum));


end


%% ########################################################################
% 生成Excel
% ########################################################################

function generateExcel(fig,testCaseNum)


%% ========================================================================
% 获取表格
% =========================================================================

tbl = findobj(fig,'Tag','SignalTable');

if isempty(tbl)

    uialert( ...
        fig, ...
        '找不到输入信号配置表。', ...
        '内部错误');

    return;
end


tableData = tbl.Data;

signalNum = size(tableData,1);


%% ========================================================================
% 创建配置数组
% =========================================================================

signalNames = strings(signalNum,1);
dataTypes = strings(signalNum,1);
typeDefinitions = strings(signalNum,1);

minValues = nan(signalNum,1);
maxValues = nan(signalNum,1);


%% ========================================================================
% 读取GUI配置
% =========================================================================

for i = 1:signalNum


    % ------------------------------------------------------------
    % Signal Name
    % ------------------------------------------------------------

    signalNames(i) = ...
        strtrim(string(tableData{i,1}));


    if signalNames(i) == ""

        uialert( ...
            fig, ...
            sprintf('第 %d 个信号名称不能为空！',i), ...
            '输入错误');

        return;
    end


    % ------------------------------------------------------------
    % Data Type
    % ------------------------------------------------------------

    dataTypes(i) = ...
        strtrim(string(tableData{i,2}));


    if dataTypes(i) == ""

        uialert( ...
            fig, ...
            sprintf('第 %d 个信号的数据类型不能为空！',i), ...
            '输入错误');

        return;
    end


    % ------------------------------------------------------------
    % Type Definition
    % ------------------------------------------------------------

    typeDefinitions(i) = ...
        strtrim(string(tableData{i,3}));


    % ------------------------------------------------------------
    % Min / Max
    % ------------------------------------------------------------

    if isSpecialType(dataTypes(i))

        minValues(i) = NaN;
        maxValues(i) = NaN;

    else

        minValues(i) = ...
            str2double(strtrim(string(tableData{i,4})));

        maxValues(i) = ...
            str2double(strtrim(string(tableData{i,5})));


        if isnan(minValues(i)) || isnan(maxValues(i))

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 的 Min/Max 必须是有效数字！', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end


        if minValues(i) > maxValues(i)

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 的 Min Value 不能大于 Max Value！', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end

    end

end


%% ========================================================================
% 信号名称统一使用 string
%
% 解决 MATLAB R2025b：
%
% unique(cell)
%
% 导致：
%
% matlab.internal.math.uniqueCellsstrHelper
%
% 的问题。
% =========================================================================

signalNames = string(signalNames);


%% ========================================================================
% 检查名称重复
% =========================================================================

if numel(unique(signalNames)) ~= signalNum

    uialert( ...
        fig, ...
        'Signal Name 不能重复！', ...
        '输入错误');

    return;
end


%% ========================================================================
% MATLAB合法变量名
% =========================================================================

validNames = matlab.lang.makeValidName(signalNames);


if numel(unique(validNames)) ~= signalNum

    uialert( ...
        fig, ...
        ['信号名称经过MATLAB变量名转换后发生重复。' newline ...
         '例如两个名称可能都被转换成同一个变量名。' newline ...
         '请重新命名。'], ...
        '信号名称错误');

    return;
end


%% ========================================================================
% 检查数据类型
% =========================================================================

for i = 1:signalNum

    type = dataTypes(i);


    % ------------------------------------------------------------
    % Built-in
    % ------------------------------------------------------------

    if isBuiltInType(type)

        continue;

    end


    % ------------------------------------------------------------
    % Fixed Point
    % ------------------------------------------------------------

    if type == "Fixed Point"

        if typeDefinitions(i) == ""

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 是 Fixed Point，但没有填写 Type Definition。', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end

        continue;

    end


    % ------------------------------------------------------------
    % Enum
    % ------------------------------------------------------------

    if type == "Enumerated"

        if typeDefinitions(i) == ""

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 是 Enumerated，但没有填写枚举类名。', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end

        continue;

    end


    % ------------------------------------------------------------
    % Bus
    % ------------------------------------------------------------

    if type == "Bus"

        if typeDefinitions(i) == ""

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 是 Bus，但没有填写Bus对象名称。', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end

        continue;

    end


    % ------------------------------------------------------------
    % ValueType
    % ------------------------------------------------------------

    if type == "ValueType"

        if typeDefinitions(i) == ""

            uialert( ...
                fig, ...
                sprintf( ...
                '%s 是 ValueType，但没有填写ValueType名称。', ...
                signalNames(i)), ...
                '输入错误');

            return;
        end

        continue;

    end


    uialert( ...
        fig, ...
        sprintf( ...
        '%s 的数据类型 %s 不支持。', ...
        signalNames(i),type), ...
        '数据类型错误');

    return;

end


%% ========================================================================
% 随机种子
% =========================================================================

rng('shuffle');


%% ========================================================================
% 创建测试用例ID
% =========================================================================

TestCase_ID = (1:testCaseNum)';


%% ========================================================================
% 创建TestCases Table
% =========================================================================

resultTable = table(TestCase_ID);


%% ========================================================================
% 生成测试数据
% =========================================================================

for i = 1:signalNum

    type = dataTypes(i);

    typeDef = typeDefinitions(i);

    minVal = minValues(i);

    maxVal = maxValues(i);


    %% ================================================================
    % double
    % ================================================================

    if type == "double"

        values = ...
            minVal + ...
            (maxVal-minVal).*rand(testCaseNum,1);


        resultTable.(validNames(i)) = values;


    %% ================================================================
    % single
    % ================================================================

    elseif type == "single"

        values = single( ...
            minVal + ...
            (maxVal-minVal).*rand(testCaseNum,1));


        resultTable.(validNames(i)) = values;


    %% ================================================================
    % half
    % ================================================================

    elseif type == "half"

        values = ...
            minVal + ...
            (maxVal-minVal).*rand(testCaseNum,1);


        try

            values = half(values);

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['half数据类型创建失败。\n\n' ...
                 '%s'],ME.message), ...
                'half错误');

            return;

        end


        % Excel没有真正的half类型
        % 所以保存real value
        resultTable.(validNames(i)) = double(values);


    %% ================================================================
    % Boolean
    % ================================================================

    elseif type == "boolean"

        values = ...
            logical(randi([0 1],testCaseNum,1));


        resultTable.(validNames(i)) = values;


    %% ================================================================
    % String
    % ================================================================

    elseif type == "string"

        values = strings(testCaseNum,1);


        for k = 1:testCaseNum

            values(k) = ...
                sprintf('Test_%05d',k);

        end


        resultTable.(validNames(i)) = values;


    %% ================================================================
    % Integer
    % ================================================================

    elseif isIntegerType(type)

        try

            values = ...
                generateIntegerData( ...
                type, ...
                minVal, ...
                maxVal, ...
                testCaseNum);

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['整数数据生成失败：\n\n%s'], ...
                ME.message), ...
                '整数错误');

            return;

        end


        resultTable.(validNames(i)) = values;


    %% ================================================================
    % Fixed Point
    % ================================================================

    elseif type == "Fixed Point"

        try

            values = ...
                generateFixedPointData( ...
                typeDef, ...
                minVal, ...
                maxVal, ...
                testCaseNum);


            % Excel写入real-world value
            resultTable.(validNames(i)) = ...
                double(values);

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['Fixed Point生成失败。\n\n' ...
                 'Type Definition：%s\n\n' ...
                 '%s'], ...
                 typeDef,ME.message), ...
                'Fixed Point错误');

            return;

        end


    %% ================================================================
    % Enumerated
    % ================================================================

    elseif type == "Enumerated"

        try

            values = ...
                generateEnumData( ...
                typeDef, ...
                testCaseNum);


            % Excel中保存枚举Underlying Value
            resultTable.(validNames(i)) = ...
                double(values);

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['Enum生成失败。\n\n' ...
                 'Enum Class：%s\n\n' ...
                 '%s'], ...
                 typeDef,ME.message), ...
                'Enum错误');

            return;

        end


    %% ================================================================
    % Bus
    % ================================================================

    elseif type == "Bus"

        try

            busObject = ...
                evalin('base',char(typeDef));


            if ~isa(busObject,'Simulink.Bus')

                error( ...
                    '变量 %s 不是 Simulink.Bus 对象。', ...
                    typeDef);

            end


            % Bus特殊处理
            %
            % 不直接把整个Bus放进Excel。
            %
            % 而是展开Bus：
            %
            % Bus.Voltage
            % Bus.Current
            % Bus.Enable
            %
            % 方便后续UT重建Bus。

            resultTable = ...
                expandBusToTable( ...
                resultTable, ...
                busObject, ...
                string(typeDef), ...
                string(validNames(i)), ...
                testCaseNum);

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['Bus生成失败。\n\n' ...
                 'Bus Object：%s\n\n' ...
                 '%s'], ...
                 typeDef,ME.message), ...
                'Bus错误');

            return;

        end


    %% ================================================================
    % ValueType
    % ================================================================

    elseif type == "ValueType"

        % ValueType本身通常需要根据工程定义进一步处理。
        %
        % 当前版本：
        %   尝试从base workspace获取对象
        %
        try

            valueTypeObj = ...
                evalin('base',char(typeDef));


            if ~isa(valueTypeObj,'Simulink.ValueType')

                error( ...
                    '变量 %s 不是 Simulink.ValueType。', ...
                    typeDef);

            end


            % ValueType通常描述底层数值类型，
            % 这里尝试根据DataType生成数据。

            baseType = string(valueTypeObj.DataType);

            baseType = ...
                strrep(baseType,'Inherit: auto','double');


            tempType = ...
                classifyDataType(baseType);


            switch tempType

                case "double"

                    values = ...
                        minVal + ...
                        (maxVal-minVal).*rand(testCaseNum,1);

                case "single"

                    values = single( ...
                        minVal + ...
                        (maxVal-minVal).*rand(testCaseNum,1));

                otherwise

                    values = ...
                        minVal + ...
                        (maxVal-minVal).*rand(testCaseNum,1);

            end


            resultTable.(validNames(i)) = values;

        catch ME

            uialert( ...
                fig, ...
                sprintf( ...
                ['ValueType生成失败。\n\n' ...
                 'ValueType：%s\n\n' ...
                 '%s'], ...
                 typeDef,ME.message), ...
                'ValueType错误');

            return;

        end

    end

end


%% ========================================================================
% 选择Excel保存位置
% =========================================================================

timeString = datestr(now,'yyyymmdd_HHMMSS');


defaultFileName = ...
    sprintf( ...
    'UT_TestCases_%s.xlsx', ...
    timeString);


[fileName,pathName] = uiputfile( ...
    '*.xlsx', ...
    '保存UT测试用例Excel文件', ...
    defaultFileName);


if isequal(fileName,0)

    fprintf('用户取消保存。\n');

    return;

end


fullFileName = ...
    fullfile(pathName,fileName);


%% ========================================================================
% TestCases Sheet
% =========================================================================

writetable( ...
    resultTable, ...
    fullFileName, ...
    'Sheet','TestCases');


%% ========================================================================
% Config Sheet
% =========================================================================

Config = table( ...
    signalNames, ...
    dataTypes, ...
    typeDefinitions, ...
    minValues, ...
    maxValues, ...
    'VariableNames', ...
    {'SignalName', ...
     'DataType', ...
     'TypeDefinition', ...
     'MinValue', ...
     'MaxValue'});


writetable( ...
    Config, ...
    fullFileName, ...
    'Sheet','Config');


%% ========================================================================
% README Sheet
% =========================================================================

Description = {
    'MATLAB / Simulink UT Test Case File'
    ''
    'Sheet 1：TestCases'
    '保存随机生成的UT输入数据。'
    ''
    'Sheet 2：Config'
    '保存每个信号的Simulink数据类型及其配置。'
    ''
    'Sheet 3：README'
    '保存测试文件说明。'
    ''
    '支持Built-in：'
    'double / single / half'
    'int8 / uint8 / int16 / uint16'
    'int32 / uint32 / int64 / uint64'
    'boolean / string'
    ''
    '支持Fixed Point：'
    'fixdt(1,16,8)'
    ''
    '支持Enumerated：'
    'Enum Class'
    ''
    '支持Bus：'
    'Simulink.Bus Object'
    ''
    '支持ValueType：'
    'Simulink.ValueType'
    ''
    '注意：Excel不能保存Simulink真实数据类型。'
    'UT导入时必须根据Config重新进行数据类型转换。'
    ''
    'Bus信号会展开为Bus.Element形式写入TestCases。'
    'Enum信号在Excel中保存Underlying Value。'
    'Fixed Point在Excel中保存Real World Value。'
    };


writecell( ...
    Description, ...
    fullFileName, ...
    'Sheet','README');


%% ========================================================================
% 关闭窗口
% =========================================================================

close(fig);


%% ========================================================================
% 输出结果
% =========================================================================

fprintf('\n');
fprintf('============================================================\n');
fprintf('UT测试用例生成完成！\n');
fprintf('文件：%s\n',fullFileName);
fprintf('测试用例数量：%d\n',testCaseNum);
fprintf('输入信号数量：%d\n',signalNum);
fprintf('============================================================\n');


msgbox( ...
    sprintf( ...
    ['UT测试用例生成完成！\n\n' ...
     '文件：\n%s\n\n' ...
     '测试用例数量：%d\n' ...
     '输入信号数量：%d'], ...
     fullFileName, ...
     testCaseNum, ...
     signalNum), ...
    '生成完成');


end


%% ########################################################################
% 判断是否是特殊数据类型
% ########################################################################

function result = isSpecialType(type)

result = ismember( ...
    type, ...
    ["string", ...
     "boolean", ...
     "Fixed Point", ...
     "Enumerated", ...
     "Bus", ...
     "ValueType"]);

end


%% ########################################################################
% 判断Built-in类型
% ########################################################################

function result = isBuiltInType(type)

result = ismember( ...
    type, ...
    ["double", ...
     "single", ...
     "half", ...
     "int8", ...
     "uint8", ...
     "int16", ...
     "uint16", ...
     "int32", ...
     "uint32", ...
     "int64", ...
     "uint64", ...
     "boolean", ...
     "string"]);

end


%% ########################################################################
% 判断整数类型
% ########################################################################

function result = isIntegerType(type)

result = ismember( ...
    type, ...
    ["int8","uint8", ...
     "int16","uint16", ...
     "int32","uint32", ...
     "int64","uint64"]);

end


%% ########################################################################
% 整数随机数据生成
% ########################################################################

function values = ...
    generateIntegerData(type,minVal,maxVal,N)


low = ceil(minVal);
high = floor(maxVal);


if low > high

    error('整数Min/Max范围没有可生成的整数。');

end


switch type

    case "uint8"

        values = uint8( ...
            randi([low high],N,1));


    case "int8"

        values = int8( ...
            randi([low high],N,1));


    case "uint16"

        values = uint16( ...
            randi([low high],N,1));


    case "int16"

        values = int16( ...
            randi([low high],N,1));


    case "uint32"

        values = uint32( ...
            randi([low high],N,1));


    case "int32"

        values = int32( ...
            randi([low high],N,1));


    case "uint64"

        % 常规汽车软件UT范围下足够。
        values = uint64( ...
            floor( ...
            low + ...
            (high-low+1).*rand(N,1)));


    case "int64"

        values = int64( ...
            floor( ...
            low + ...
            (high-low+1).*rand(N,1)));


    otherwise

        error('Unsupported integer type.');

end

end


%% ########################################################################
% Fixed Point生成
% ########################################################################

function values = ...
    generateFixedPointData(typeDef,minVal,maxVal,N)


%% 尝试解析fixdt

try

    numericType = eval(typeDef);

catch ME

    error( ...
        '无法解析Fixed Point表达式：%s\n%s', ...
        typeDef,ME.message);

end


if ~isa(numericType,'Simulink.NumericType')

    error( ...
        'Type Definition必须是有效的Simulink.NumericType，例如 fixdt(1,16,8)。');

end


%% 生成double随机数

rawValues = ...
    minVal + ...
    (maxVal-minVal).*rand(N,1);


%% 获取属性

signed = numericType.SignednessBool;

wordLength = numericType.WordLength;

fractionLength = numericType.FractionLength;


%% 使用fi量化

values = fi( ...
    rawValues, ...
    signed, ...
    wordLength, ...
    fractionLength, ...
    'RoundingMethod','Nearest', ...
    'OverflowAction','Saturate');


end


%% ########################################################################
% Enum生成
% ########################################################################

function values = ...
    generateEnumData(enumClass,N)


%% 检查类是否存在

if exist(enumClass,'class') ~= 8

    error( ...
        '找不到Enum Class：%s。请确认该类已经加入MATLAB路径。', ...
        enumClass);

end


%% 获取枚举成员

try

    memberInfo = enumeration(enumClass);

catch ME

    error( ...
        '无法读取Enum成员：%s', ...
        ME.message);

end


if isempty(memberInfo)

    error( ...
        'Enum Class %s 没有枚举成员。', ...
        enumClass);

end


%% 随机选择Enum成员

index = ...
    randi([1 numel(memberInfo)],N,1);


%% 创建输出

values = ...
    memberInfo(index);

end


%% ########################################################################
% Bus展开
% ########################################################################

function resultTable = ...
    expandBusToTable( ...
    resultTable, ...
    busObject, ...
    prefix, ...
    variablePrefix, ...
    N)


%% 获取Bus Elements

elements = busObject.Elements;


for i = 1:numel(elements)

    element = elements(i);


    elementName = string(element.Name);


    currentPrefix = ...
        variablePrefix + "." + elementName;


    %% ------------------------------------------------------------
    % 如果还是Bus
    % ------------------------------------------------------------

    dataType = string(element.DataType);


    if startsWith(dataType,"Bus:")

        nestedBusName = ...
            strtrim( ...
            extractAfter(dataType,"Bus:"));


        nestedBus = ...
            evalin('base',char(nestedBusName));


        resultTable = ...
            expandBusToTable( ...
            resultTable, ...
            nestedBus, ...
            currentPrefix, ...
            currentPrefix, ...
            N);


        continue;

    end


    %% ------------------------------------------------------------
    % 普通Bus Element
    % ------------------------------------------------------------

    columnName = ...
        matlab.lang.makeValidName(currentPrefix);


    %% 从Bus Element读取Min/Max

    minVal = element.Min;

    maxVal = element.Max;


    if isempty(minVal)

        minVal = -100;

    end


    if isempty(maxVal)

        maxVal = 100;

    end


    %% DataType处理

    baseType = ...
        classifyDataType(dataType);


    switch baseType

        case "double"

            if ~isfinite(minVal)
                minVal = -100;
            end

            if ~isfinite(maxVal)
                maxVal = 100;
            end

            values = ...
                minVal + ...
                (maxVal-minVal).*rand(N,1);


        case "single"

            if ~isfinite(minVal)
                minVal = -100;
            end

            if ~isfinite(maxVal)
                maxVal = 100;
            end

            values = single( ...
                minVal + ...
                (maxVal-minVal).*rand(N,1));


        case "boolean"

            values = logical( ...
                randi([0 1],N,1));


        case "int8"

            values = int8( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        case "uint8"

            values = uint8( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        case "int16"

            values = int16( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        case "uint16"

            values = uint16( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        case "int32"

            values = int32( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        case "uint32"

            values = uint32( ...
                randi([ceil(minVal) floor(maxVal)],N,1));


        otherwise

            % 对复杂Bus Element：
            % 先保存double，后续Config中保留原DataType。
            values = ...
                minVal + ...
                (maxVal-minVal).*rand(N,1);

    end


    %% 添加到TestCases

    resultTable.(columnName) = values;

end

end


%% ########################################################################
% 数据类型分类
% ########################################################################

function type = classifyDataType(dataType)


dataType = string(strtrim(dataType));


if dataType == "double"

    type = "double";

elseif dataType == "single"

    type = "single";

elseif dataType == "boolean"

    type = "boolean";

elseif dataType == "int8"

    type = "int8";

elseif dataType == "uint8"

    type = "uint8";

elseif dataType == "int16"

    type = "int16";

elseif dataType == "uint16"

    type = "uint16";

elseif dataType == "int32"

    type = "int32";

elseif dataType == "uint32"

    type = "uint32";

elseif dataType == "int64"

    type = "int64";

elseif dataType == "uint64"

    type = "uint64";

elseif dataType == "half"

    type = "half";

elseif startsWith(dataType,"fixdt(")

    type = "fixed";

elseif startsWith(dataType,"Enum:")

    type = "enum";

elseif startsWith(dataType,"Bus:")

    type = "bus";

else

    type = "double";

end

end