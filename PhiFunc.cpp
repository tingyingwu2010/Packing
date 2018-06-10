class PhiFuncPrim;

tuple<point,point> moveLine(point,point,double);

class PhiFunc{
        static double sign(double x){return ((x>=0) ? 1 : -1);}
    public:
        virtual double eval(const double* x) = 0;
        virtual vector<PhiFuncPrim*> getIneqs(const double* x) = 0;
        virtual vector<int> getIndices(const double* x) = 0;

        static PhiFunc* phiFunc(point,point,RotTrans,point ,RotTrans);
        static PhiFunc* phiFunc(point,point,RotTrans,circle,RotTrans);
        static PhiFunc* phiFunc(circle,RotTrans,point ,RotTrans);
        static PhiFunc* phiFunc(circle,RotTrans,circle,RotTrans);
        static PhiFunc* phiFunc(circle,Scale,point ,RotTrans);
        static PhiFunc* phiFunc(circle,Scale,circle,RotTrans);
        static PhiFunc* phiFunc(circle,RotTrans,circle,point,RotTrans,double);
        static PhiFunc* phiFunc(circle,Scale   ,circle,point,RotTrans,double);
};

class PhiFuncPrim: public PhiFunc{
    public:
        virtual void getD1(const double* x,double* res) = 0;
        virtual vector<int> getD1ind() = 0;
        virtual void getD2(const double* x,double* res) = 0;
        virtual vector<tuple<int,int>> getD2ind() = 0;
        vector<PhiFuncPrim*> getIneqs (const double* x){return {this};}
        vector<int> getIndices (const double* x){return {};}
};

class PhiFuncNode: public PhiFunc{
    private:
        bool sense; //0 -> max,1 -> min
        vector<PhiFunc*> nodes;

        int argmax(const double* x){
            int ind = 0;
            double val = nodes[0]->eval(x);
            for(int i=1;i<nodes.size();i++){
                double tmp = nodes[i]->eval(x);
                if(tmp>val){
                    val = tmp;
                    ind = i;
                }
            }
            return ind;
        }

    public:
        PhiFuncNode(bool s,vector<PhiFunc*> n) : sense(s),nodes(n) {};

        double eval(const double* x){
            double res = nodes[0]->eval(x);
            for(int i=1;i<nodes.size();i++){
                double tmp = nodes[i]->eval(x);
                if(sense ^ (tmp>res))
                    res=tmp;
            }
            return res;
        }

        vector<PhiFuncPrim*> getIneqs(const double* x){
            if(sense){
                vector<PhiFuncPrim*> res = {};
                for(int i=0;i<nodes.size();i++){
                    vector<PhiFuncPrim*> tmp = nodes[i]->getIneqs(x);
                    res.insert(res.end(),tmp.begin(),tmp.end());
                }
                return res;
            } else
                return nodes[argmax(x)]->getIneqs(x);
        }

        vector<int> getIndices(const double* x){
            if(sense){
                vector<int> res = {};
                for(int i=0;i<nodes.size();i++){
                    vector<int> tmp = nodes[i]->getIndices(x);
                    res.insert(res.end(),tmp.begin(),tmp.end());
                }
                return res;
            } else {
                int ind = argmax(x);
                vector<int> res = nodes[ind]->getIndices(x);
                res.push_back(ind);
                return res;
            }
        }
};

class PhiFuncCcScClRt : public PhiFuncPrim{ //Only unit-circle around (0,0)
    double px,py,rc;
    double d1,d2;
    int i,j;
  public:
    PhiFuncCcScClRt(circle cc,Scale f,circle c,RotTrans g){
        px = c.p.x; py = c.p.y; rc = c.r;
        i = f.ind; j = g.ind;
    }

    double eval(const double* x){
        double s = sin(x[j+2]), c = cos(x[j+2]);
        double dr = x[i]  -rc;
        double dx = x[j]  +px*c+py*s;
        double dy = x[j+1]-px*s+py*c;
        return dr*dr-dx*dx-dy*dy;
    }

    vector<int> getD1ind(){
        return {i,j,j+1,j+2};
    }

    void getD1(const double* x,double* res){
        double s = sin(x[j+2]), c = cos(x[j+2]);
        res[0] = 2*(x[i]-rc);
        res[1] =-2*(x[j]  +px*c+py*s);
        res[2] =-2*(x[j+1]-px*s+py*c);
        res[3] = 2*(x[j]*(px*s-py*c)+x[j+1]*(px*c+py*s));
    }

    vector<tuple<int,int>> getD2ind(){
        return {{i,i},{j,j},{j+1,j+1},{j,j+2},{j+1,j+2},{j+2,j+2}};
    }

    void getD2(const double* x,double* res){
        double s = sin(x[j+2]), c = cos(x[j+2]);
        res[0] = 2; res[1] = -2; res[2] = -2;
        res[3] = 2*(px*s-py*c);
        res[4] = 2*(px*c+py*s);
        res[5] = 2*(x[j]*(px*c+py*s)+x[j+1]*(py*c-px*s));
    }
};

class PhiFuncHCcScCsRt : public PhiFuncPrim{
    double dx,dy,det;
    int i;
  public:
    PhiFuncHCcScCsRt(RotTrans f, circle c, point p, double s){
      dx = s*(c.p.y-p.y);
      dy = s*(p.x-c.p.x);
      det= s*(p.x*c.p.y-p.y*c.p.x);
      i = f.ind;
    }

    double eval(const double* x){
        double s = sin(x[i+2]), c = cos(x[i+2]);
        return x[i]*(dx*c+dy*s)+x[i+1]*(-dx*s+dy*c)+det;
    }

    vector<int> getD1ind(){
        return {i,i+1,i+2};
    }

    void getD1(const double* x,double* res){
        double s = sin(x[i+2]), c = cos(x[i+2]);
        res[0] =  dx*c+dy*s;
        res[1] = -dx*s+dy*c;
        res[2] = x[i]*(dy*c-dx*s)-x[i+1]*(dx*c+dy*s);
    }

    vector<tuple<int,int>> getD2ind(){
        return {{i,i+2},{i+1,i+2},{i+2,i+2}};
    }

    void getD2(const double* x,double* res){
        double s = sin(x[i+2]), c = cos(x[i+2]);
        res[0] = -dx*s+dy*c;
        res[1] = -dx*c-dy*s;
        res[2] = -x[i]*(dy*s+dx*c)+x[i+1]*(dx*s-dy*c);
    }
};
//(Could be templatized with matrix arguments)
class PhiFuncHCcRtCsRt : public PhiFuncPrim{
    Matrix2d A,B;
    double c;
    int i,j;
  public:
    PhiFuncHCcRtCsRt(circle cc,RotTrans f,circle pc,point p,RotTrans g,double s){
        double ccx = cc.p.x, ccy = cc.p.y, pcx = pc.p.x, pcy = pc.p.y;
        A << pcx-p.x, p.y-pcy, pcy-p.y, pcx-p.x;
        B << ccy*(p.x-pcx)+ccx*(pcy-p.y), ccx*(pcx-p.x)+ccy*(pcy-p.y),
             ccx*(p.x-pcx)+ccy*(p.y-pcy), ccy*(p.x-pcx)+ccx*(pcy-p.y);
        c = pcx*p.y-pcy*p.x;
        A*= s; B*= s; c*= s;
        i = f.ind; j = g.ind;
    }

    double eval(const double* x){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2])),y(x[j]-x[i],x[j+1]-x[i+1]);
        Vector2d f2(sin(x[j+2]),cos(x[j+2]));
        return (y*A+f1*B)*f2+c;
    }

    vector<int> getD1ind(){
        return {i,i+1,i+2,j,j+1,j+2};
    }

    void getD1(const double* x,double* res){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        Vector2d    f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        RowVector2d y(x[j]-x[i],x[j+1]-x[i+1]);
        Vector2d    Af2 = A*f2;
        double tmp[] = {-Af2[0],-Af2[1],f1d*B*f2,Af2[0],Af2[1],(y*A+f1*B)*f2d};
        for(int i=0;i<6;i++) res[i] = tmp[i];
    }

    vector<tuple<int,int>> getD2ind(){
        if(i<j)
            return {{i+2,i+2},{j,j+2},{j+1,j+2},{j+2,j+2},{i,j+2},{i+1,j+2},{i+2,j+2}};
        else
            return {{i+2,i+2},{j,j+2},{j+2,j+2},{j+2,j+2},{j+2,i},{j+2,i+1},{j+2,i+2}};
    }

    void getD2(const double* x,double* res){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        Vector2d    f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        RowVector2d y(x[j]-x[i],x[j+1]-x[i+1]);
        Vector2d    Af2d = A*f2d;
        double f1Bf2 = -f1*B*f2;
        double tmp[] {f1Bf2,Af2d[0],Af2d[1],f1Bf2,-Af2d[0],-Af2d[1],f1d*B*f2d};
        for(int i=0;i<7;i++) res[i] = tmp[i];
    }
};

class PhiFuncLnRtClRt : public PhiFuncPrim{
    Matrix2d A,B,T;
    double c;
    int i,j;
  public:
    PhiFuncLnRtClRt(point l1, point l2, RotTrans f, point p, RotTrans g){
        double dx=l1.x-l2.x,dy=l1.y-l2.y,n=1/sqrt(dx*dx+dy*dy);
        dx *= n; dy *= n;
        A << dx*p.y-dy*p.x, -dy*p.y-dx*p.x, dx*p.x+dy*p.y, dx*p.y-dy*p.x;
        B << dx, -dy, dy, dx;
        c = n*(l2.x*l1.y-l1.x*l2.y);
        i = f.ind; j = g.ind;
    }
    
    double eval(const double* x){
        Vector2d    f1(sin(x[i+2]),cos(x[i+2]));
        RowVector2d f2(sin(x[j+2]),cos(x[j+2])),y(x[j]-x[i],x[j+1]-x[i+1]);
        return (f2*A+y*B)*f1+c;
    }

    vector<int> getD1ind(){
        return {i,i+1,i+2,j,j+1,j+2};
    }

    void getD1(const double* x,double* res){
        Vector2d    f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        RowVector2d f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        RowVector2d y(x[j]-x[i],x[j+1]-x[i+1]);
        Vector2d    Bf1 = B*f1;
        double tmp[] = {-Bf1[0],-Bf1[1],(f2*A+y*B)*f1d,Bf1[0],Bf1[1],f2d*A*f1};
        for(int i=0;i<6;i++) res[i] = tmp[i];
    }

    vector<tuple<int,int>> getD2ind(){
        if(i<j)
            return {{i,i+2},{i+1,i+2},{i+2,i+2},{j+2,j+2},{i+2,j},{i+2,j+1},{i+2,j+2}};
        else
            return {{i,i+2},{i+1,i+2},{i+2,i+2},{j+2,j+2},{j,i+2},{j+1,i+2},{j+2,i+2}};
    }

    void getD2(const double* x,double* res){
        Vector2d    f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        RowVector2d f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        RowVector2d y(x[j]-x[i],x[j+1]-x[i+1]);
        Vector2d    Bf1d = B*f1d;
        RowVector2d f2A=f2*A;
        double tmp[] {-Bf1d[0],-Bf1d[1],-f2A*f1,-(f2A+y*B)*f1,Bf1d[0],Bf1d[1],f2d*A*f1d};
        for(int i=0;i<7;i++) res[i] = tmp[i];
    }
};

class PhiFuncClRtClRt : public PhiFuncPrim{
    Matrix2d A,R1,R2;
    double c,s;
    int i,j;
  public:
    PhiFuncClRtClRt(circle c1, RotTrans f, circle c2, RotTrans g, double o = 1){
        double x1 = c1.p.x, y1 = c1.p.y, x2 = c2.p.x, y2 = c2.p.y, r = c1.r+c2.r;
        A  << -2*(x1*x2+y1*y2), 2*(x1*y2-x2*y1), 2*(x2*y1-x1*y2), -2*(x1*x2+y1*y2);
        R1 <<  2*y1,-2*x1, 2*x1, 2*y1;
        R2 << -2*y2,-2*x2, 2*x2,-2*y2;
        c = x1*x1+x2*x2+y1*y1+y2*y2-r*r;
        s = o; A*=o; R1*=o; R2*=o; c*=o;
        i = f.ind; j = g.ind;
    }
    
    PhiFuncClRtClRt(circle c, RotTrans f, point p, RotTrans g, double o = 1) :
        PhiFuncClRtClRt(c,f,circle(p,0),g,o) {}

    double eval(const double* x){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2]));
        Vector2d    f2(sin(x[j+2]),cos(x[j+2]));
        Vector2d     y(x[i]-x[j],x[i+1]-x[j+1]);
        return y.dot(s*y+R2*f2)+f1*(R1*y+A*f2)+c;
    }

    vector<int> getD1ind(){
        return {i,i+1,i+2,j,j+1,j+2};
    }

    void getD1(const double* x,double* res){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        Vector2d    f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        Vector2d     y(x[i]-x[j],x[i+1]-x[j+1]);
        Vector2d dy = 2*s*y+(f1*R1).transpose()+R2*f2;
        double tmp[] {dy[0],dy[1],f1d*(R1*y+A*f2),-dy[0],-dy[1],y.dot(R2*f2d)+f1*A*f2d};
        for(int i=0;i<6;i++) res[i] = tmp[i];
    }

    vector<tuple<int,int>> getD2ind(){
        if(i<j)
            return {{i,i},{i+1,i+1},{i,i+2},{i+1,i+2},{i+2,i+2},
                    {j,j},{j+1,j+1},{j,j+2},{j+1,j+2},{j+2,j+2},
                    {i,j},{i+1,j+1},{i,j+2},{i+1,j+2},{i+2,j},{i+2,j},{i+2,j+2}};
        else
            return {{i,i},{i+1,i+1},{i,i+2},{i+1,i+2},{i+2,i+2},
                    {j,j},{j+1,j+1},{j,j+2},{j+1,j+2},{j+2,j+2},
                    {j,i},{j+1,i+1},{j,i+2},{j+1,i+2},{j+2,i},{j+2,i},{j+2,i+2}};
    }

    void getD2(const double* x,double* res){
        RowVector2d f1(sin(x[i+2]),cos(x[i+2])),f1d(f1[1],-f1[0]);
        Vector2d    f2(sin(x[j+2]),cos(x[j+2])),f2d(f2[1],-f2[0]);
        Vector2d     y(x[i]-x[j],x[i+1]-x[j+1]);
        RowVector2d dyf1 = f1d*R1;
        Vector2d    dyf2 = R2*f2d;
        double fAf= -f1*A*f2;
        double tmp[] {2*s, 2*s, dyf1[0], dyf1[1],         -f1*R1*y+fAf,
                      2*s, 2*s, dyf2[0], dyf2[1],    -y.dot(R2*f2)+fAf,
                     -2*s,-2*s,-dyf1[0],-dyf1[1],-dyf2[0],-dyf2[1],fAf};
    }
};

PhiFunc* PhiFunc::phiFunc(point p0,point p1,RotTrans f,point q,RotTrans g){
    return new PhiFuncLnRtClRt(p0,p1,f,q,g);
}

PhiFunc* PhiFunc::phiFunc(point p0,point p1,RotTrans f,circle c,RotTrans g){
    tuple<point,point> tmp = moveLine(p0,p1,c.r);
    return new PhiFuncLnRtClRt(get<0>(tmp),get<1>(tmp),f,c.p,g);
}

PhiFunc* PhiFunc::phiFunc(circle c,RotTrans f,point p,RotTrans g){
    return new PhiFuncClRtClRt(c,f,p,g,sign(c.r));
}
//If c1<0 then -c1>c2
PhiFunc* PhiFunc::phiFunc(circle c1,RotTrans f,circle c2,RotTrans g){
    return new PhiFuncClRtClRt(circle(c1.p,c1.r+c2.r),f,c2.p,g,sign(c1.r));
}

//OK for scaling, otherwise negative circle radius determines complement
PhiFunc* PhiFunc::phiFunc(circle cc,Scale f,point p,RotTrans g){
    return new PhiFuncCcScClRt(cc,f,circle(p,0),g);
}

PhiFunc* PhiFunc::phiFunc(circle cc,Scale f,circle c,RotTrans g){
    return new PhiFuncCcScClRt(cc,f,c,g);
}

PhiFunc* PhiFunc::phiFunc(circle cc,RotTrans f,circle pc,point p,RotTrans g,double s){
    return new PhiFuncHCcRtCsRt(cc,f,pc,p,g,s);
}

PhiFunc* PhiFunc::phiFunc(circle cc,Scale    f,circle pc,point p,RotTrans g,double s){
    return new PhiFuncHCcScCsRt(g,pc,p,s);
}
