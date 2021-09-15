#include "getHan.h"
using namespace cv;
struct P
{
	int idx;
	float val;
	bool operator < (const P & a) const 
	{
		return a.val < val;
	}	
};
Rect getRect(Mat m){
    uchar *row,*col;
    int ux=-1,uy=-1,dx=-1,dy=-1;
    for (int i=0;i<m.rows;i++){
        row=m.ptr<uchar>(i);
        for (int j=0;j<m.cols;j++)
            if (row[j]==0){
                uy=i;
                break;
            }
        if (uy!=-1) break;
    }
    for (int i=m.rows-1;i>=0;i--){
        row=m.ptr<uchar>(i);
        for (int j=0;j<m.cols;j++)
            if (row[j]==0){
                dy=i;
                break;
            }
        if (dy!=-1) break;
    }
    for (int i=0;i<m.cols;i++){
        for (int j=0;j<m.rows;j++)
            if (m.ptr<uchar>(j)[i]==0){
                ux=i;
                break;
            }
        if (ux!=-1) break;
    }
    for (int i=m.cols-1;i>=0;i--){
        for (int j=0;j<m.rows;j++)
            if (m.ptr<uchar>(j)[i]==0){
                dx=i;
                break;
            }
        if (dx!=-1) break;
    }
    int dis,dt=dx-ux-(dy-uy);
    if (dt>0){
        while (dy-uy==dx-ux){
            if (uy>0) uy--;
            if (dy-uy==dx-ux) break;
            if (dy<m.cols) dy++;
        }
    }
    else{
        while (dy-uy==dx-ux){
            if (ux>0) ux--;
            if (dy-uy==dx-ux) break;
            if (dx<m.cols) dx++;
        }
    }
    printf("%d %d\n", dx-ux,dy-uy);
    return Rect(ux,uy,dx-ux,dy-uy);
}

vector<int> gethan(Mat image)
{
	P p[1000];
	vector<Mat> fen;
	split(image,fen);
	Mat des(64, 64, fen[0].type());
    fen[0]=fen[0](getRect(fen[0]));
	resize(fen[0],des,des.size(),0,0,CV_INTER_LINEAR);
	vector<float> input_vals= (vector<float>)(des.reshape(1,1));
	for(int i=0;i<input_vals.size();i++)
		input_vals[i]/=255.0;

    auto graph = getGraph();
    const std::vector<std::int64_t> input_dims1 = {1,64, 64, 1};
    auto input_tensor1 = tf_utils::CreateTensor(TF_FLOAT, input_dims1, input_vals);  //make tensor
    SCOPE_EXIT{ tf_utils::DeleteTensor(input_tensor1); }; // Auto-delete on scope exit.
      
    const std::vector<std::int64_t> input_dims2 ={1};
    vector<float> d={1};
    auto input_tensor2 = tf_utils::CreateTensor(TF_FLOAT,input_dims2 ,d);  //make tensor
    SCOPE_EXIT{ tf_utils::DeleteTensor(input_tensor2); }; 
    vector<TF_Tensor*>  input_tensors;
    input_tensors.push_back(input_tensor1);     
    input_tensors.push_back(input_tensor2); 	

    auto input_op1 = TF_Output{TF_GraphOperationByName(graph, "img_batch"), 0};
    auto input_op2 = TF_Output{TF_GraphOperationByName(graph, "keep_prob"), 0};
    vector<TF_Output>  inputops;
    inputops.push_back(input_op1);
    inputops.push_back(input_op2);	  


    auto out_op = TF_Output{TF_GraphOperationByName(graph, "Softmax"), 0};
    vector<TF_Output>  outputops;
    outputops.push_back(out_op);

    TF_Tensor* output_tensor = nullptr;
    vector<TF_Tensor*>  output_tensors;
    output_tensors.push_back(output_tensor);
    	
    auto session = getSession();
    auto code = tf_utils::RunSession(session, inputops, input_tensors, outputops,output_tensors);
    if (code == TF_OK) {
        auto result = tf_utils::GetTensorData<float>(output_tensors[0]);
        for(int i=0;i<result.size();i++)
        {
        	p[i].idx=i;
        	p[i].val=result[i];
        }
    } 
    else {
        std::cout << "Error run session TF_CODE: " << code;
    }
	sort(p,p+999);
	vector<int> res;
    for(int i=0;i<40;i++)
	   res.push_back(p[i].idx);
	return res;
}
vector<int> getEnglish(Mat image)
{
    P p[100];
    vector<Mat> fen;
    split(image,fen);
    Mat des(64, 64, fen[0].type());
    //fen[0]=fen[0](getRect(fen[0]));
    resize(fen[0],des,des.size(),0,0,CV_INTER_LINEAR);
    vector<float> input_vals= (vector<float>)(des.reshape(1,1));
    for(int i=0;i<input_vals.size();i++)
        input_vals[i]/=255.0;
    
    auto graph = getGraphE();
    const std::vector<std::int64_t> input_dims1 = {1,64, 64, 1};
    auto input_tensor1 = tf_utils::CreateTensor(TF_FLOAT, input_dims1, input_vals);  //make tensor
    SCOPE_EXIT{ tf_utils::DeleteTensor(input_tensor1); }; // Auto-delete on scope exit.

    const std::vector<std::int64_t> input_dims2 ={1};
    vector<float> d={1};
    auto input_tensor2 = tf_utils::CreateTensor(TF_FLOAT,input_dims2 ,d);  //make tensor
    SCOPE_EXIT{ tf_utils::DeleteTensor(input_tensor2); }; 
    vector<TF_Tensor*>  input_tensors;
    input_tensors.push_back(input_tensor1);     
    input_tensors.push_back(input_tensor2);
     auto input_op1 = TF_Output{TF_GraphOperationByName(graph, "img_batch"), 0};
    auto input_op2 = TF_Output{TF_GraphOperationByName(graph, "keep_prob"), 0};
    vector<TF_Output>  inputops;
    inputops.push_back(input_op1);
    inputops.push_back(input_op2);    


    auto out_op = TF_Output{TF_GraphOperationByName(graph, "Softmax"), 0};
    vector<TF_Output>  outputops;
    outputops.push_back(out_op);

    TF_Tensor* output_tensor = nullptr;
    vector<TF_Tensor*>  output_tensors;
    output_tensors.push_back(output_tensor);
        
    auto session = getSessionE();
    auto code = tf_utils::RunSession(session, inputops, input_tensors, outputops,output_tensors);
    if (code == TF_OK) {
        auto result = tf_utils::GetTensorData<float>(output_tensors[0]);
        for(int i=0;i<result.size();i++)
        {
            p[i].idx=i;
            p[i].val=result[i];
        }
    } 
    else {
        std::cout << "Error run session TF_CODE: " << code;
    }  
    sort(p,p+47);
    vector<int> res;
    for(int i=0;i<47;i++)
    {
       // printf("%d\n",p[i].idx);
       res.push_back(p[i].idx);
    }
    return res;
}
