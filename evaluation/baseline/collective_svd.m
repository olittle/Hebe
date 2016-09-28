%input_folder='/shared/data/tensor_embedding/data/dblp/matlab/';

if ~isfield(options,'dim')
    options.dim = 300
end
if ~isfield(options, 'log')
    options.log = 0
end
if ~isfield(options, 'norm')
    options.norm = 0
end

files = dir(strcat(input_folder, 'network*.txt'));
for i=1:length(files)
    disp(files(i).name);
    path = strcat(input_folder, files(i).name);
    eval(['load ' path ' -ascii']);
    [~,name,~] = fileparts(path);
    data_{i} = spconvert(eval(name))';
end

data{1} = data_{1} * data_{2}';
for i = 3:size(data_,2)
    data{i-1} = data_{i} * data_{2}';
end

if options.norm == 1
    max_sum = sum(sum(data{1}));
    for i = 2:length(data)
        if max_sum < sum(sum(data{i}))
            max_sum = sum(sum(data{i}));
        end
    end

    for i = 1:length(data)
        data{i} = data{i} * (max_sum / sum(sum(data{i})));
    end
end

%% generate data matrix
mFea = 0;
nSmp = size(data{1}, 2);

X = sparse(mFea, nSmp);
base = 1;
for i = 1:length(data)
    X(base:(base + size(data{i}, 1) - 1), 1:nSmp) = data{i};
    base = base + size(data{i}, 1);
end

if options.log == 1
    X = spfun(@log, X);
end

[U_final, ~, V_final] = svds(X, options.dim);
%norms = sqrt(sum(V_final.^2,2));
%norms = max(norms,1e-10);
% V_final = V_final./repmat(norms,1,size(V_final,2));
path = strcat(output_folder, '/svd2.txt');
save(path,'V_final', '-ascii')
% base = 1;
% for i = 1:length(data)
%     A = V_final(base:(base + size(data{i}, 1) - 1), :);
%     % norms = sqrt(sum(A.^2,2));
%     % norms = max(norms,1e-10);
%     % A = A./repmat(norms,1,size(A,2));
%     path = strcat('results/svd', num2str(i), '.txt');
%     save(path,'A', '-ascii')
% end