#include<vector>
#include<time.h>
#include<cmath>
#include<set>

/*enum _type{*/
    //ME,
    //BALL,
    //EXP,
    //WALL,
    //PROP,
/*};*/

enum _type { 
    ME = 0x0001,
    BALL = 0x0002,
    EXP = 0x0004,
    WALL = 0x0008,
    PROP = 0x0010,
};

enum _prop_type {
    NONE,
    JIZZ,
    AVATAR,
};

struct UserData{
    _type type;
    void *data = NULL;
};


class FooDraw : public b2DebugDraw //Not defined yet
{
    public:
        void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {}
        void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {}
        void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {}
        void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {}
        void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {}
        void DrawTransform(const b2Transform& xf) {}
};

class Ball {
    public:
        int stun;
        b2Body* b_body;
        b2Body* m_body;
        float b_radius;
        int b_numContacts;
        b2Color b_color;
        Ball(b2Body* body, b2World* world, float radius, b2Color color){
	        stun  = 0;
            m_body = body;//This is Me
            b2Vec2 m_pos = body->GetPosition();
            b_numContacts = 0;
            b_body = NULL;
            b_radius = radius;
            b_color = color;
            
            b2BodyDef b_BodyDef;
            b_BodyDef.type = b2_dynamicBody;
            b2Vec2 rand_pos;
            while(1){// Enemies appear randomly
                rand_pos.x = (rand()%30)-15;
                rand_pos.y = (rand()%30)+5;
                float distance = sqrt(pow((rand_pos.x - m_pos.x), 2) + pow((rand_pos.y - m_pos.y), 2));
                if(distance > 15) break;//Enemies cannot appear within 5 unit of me
            }
            b_BodyDef.position.Set(rand_pos.x, rand_pos.y);
            b_BodyDef.linearDamping = 0.1;
            b_BodyDef.fixedRotation = true;//Set body to non-rotatable
            b_body = world->CreateBody(&b_BodyDef);
            UserData *u = new UserData;
            u->type = BALL;
            u->data = this;
            b_body->SetUserData(u); //Store the Object to UserDate          
            b2CircleShape b_circleShape; 
            b_circleShape.m_p.Set(0, 0);
            b_circleShape.m_radius = b_radius; 
            b2FixtureDef b_FixtureDef;
            b_FixtureDef.shape = &b_circleShape;
            b_FixtureDef.density = 0.3;
            b_FixtureDef.filter.categoryBits = BALL; // Collision Filter
            b_FixtureDef.filter.maskBits = BALL | WALL | EXP | ME; // Collision Filter
            b_body->CreateFixture(&b_FixtureDef);

        }
        b2Body* body(){// return body in the class
            return b_body;    
        }
        void startContact() {
		    stun = 30;
            b_numContacts++; 
        } // ++ when the object attaches to something
        void endContact() { b_numContacts--; } // -- when the object dettaches to something
        void render() { //draw a face
/*            if (b_numContacts)*/
                //glColor3f(0,1,0);// green
            //else
                //glColor3f(1,1,1);//white
            glColor3f(b_color.r, b_color.g, b_color.b);
            //nose and eyes
            glPointSize(4);
            glBegin(GL_POINTS);
            glVertex2f( 0, 0 );
            glVertex2f(-0.5, 0.5 );
            glVertex2f( 0.5, 0.5 );
            glEnd();

            //mouth
            glBegin(GL_LINES);
            glVertex2f(-0.5,  -0.5 );
            glVertex2f(-0.16, -0.6 );
            glVertex2f( 0.16, -0.6 );
            glVertex2f( 0.5,  -0.5 );
            glEnd();

            //circle outline
            glBegin(GL_LINE_LOOP);
            for (float a = 0; a < 360 * 0.0174532925; a += 30 * 0.0174532925)
                glVertex2f( sinf(a), cosf(a) );
            glEnd();
        }
        void renderAtBodyPosition() {
            b2Vec2 pos = b_body->GetPosition();
            float angle = b_body->GetAngle();

            //call normal render at different position/rotation
            glPushMatrix();
            glTranslatef( pos.x, pos.y, 0 );
            glRotatef( angle * 57.2957795, 0, 0, 1 );//OpenGL uses degrees here
            
            glScalef( b_radius, b_radius, 1 ); //add this to correct size
            render();
            render();//normal render at (0,0)
            glPopMatrix();
        }
        void moveTowardMe(){
            if(stun>0) stun--;
            else{
                float myBodyAngle = b_body->GetAngle();
                    b2Vec2 d_vel = b_body->GetLinearVelocity();
                    b2Vec2 vel = m_body->GetPosition() - b_body->GetPosition();
                    vel.Normalize();
                    vel *= 0.5;// Speed
                    if(sqrt(pow(d_vel.x, 2) + pow(d_vel.y, 2)) < 4){
                        d_vel += vel;
                    }
                    else{
                        d_vel.Normalize();
                        d_vel *= 4;
                    }
                    b_body->SetLinearVelocity(d_vel);// Set direction toward me 
            }   
        }
};


class Me{
    public:
        int health;
        b2Body* m_body;
        float m_edge;
        int m_numContacts;
        b2Color m_color;
        _prop_type prop_type;
        Me(b2World* world, float edge, b2Color color, b2Vec2 pos){
            prop_type = NONE;
            m_numContacts = 0;
            m_body = NULL;
            m_edge = edge;
            m_color = color;
            health=100;
            b2BodyDef m_BodyDef;
            
            m_BodyDef.type = b2_dynamicBody;
            m_BodyDef.position.Set(pos.x, pos.y);
            m_BodyDef.linearDamping = 0.8;
            
            m_body = world->CreateBody(&m_BodyDef);
            
            UserData *u = new UserData;
            u->type = ME;
            u->data = this;
            m_body->SetUserData(u);           
            
            b2PolygonShape m_polygonShape;
            m_polygonShape.SetAsBox(1,1);
            b2FixtureDef m_FixtureDef;
            m_FixtureDef.shape = &m_polygonShape;
            m_FixtureDef.density = 1;
            //m_FixtureDef.filter.groupIndex = -1;
            m_FixtureDef.filter.categoryBits = ME;
            m_FixtureDef.filter.maskBits = ME | BALL | WALL | PROP;
            m_body->CreateFixture(&m_FixtureDef);

        }
        b2Body* body(){// return body in the class
            return m_body;    
        }
        _prop_type prop(){
            return prop_type;
        }
        void eat_prop(_prop_type p){
            prop_type = p;
        }
        void startContact() { m_numContacts++; }
        void endContact() { m_numContacts--; }
        void render() {
        }
        void renderAtBodyPosition() {
        }
};

class Prop{
    public:
        b2Body* p_body;
        float p_radius;
        int p_numContacts;
        b2Color p_color;
        //int p_type;
        _prop_type prop_type;
        Prop(b2World* world, float radius, b2Color color, int type){
            p_numContacts = 0;
            p_body = NULL;
            p_radius = radius;
            p_color = color;
            if(type == 1){
                prop_type = JIZZ;
            }
            if(type == 2){
                prop_type = AVATAR;
            }
            b2BodyDef p_BodyDef;
            p_BodyDef.type = b2_dynamicBody;
            b2Vec2 rand_pos;
            rand_pos.x = (rand()%30)-15;
            rand_pos.y = (rand()%30)+5;
            p_BodyDef.position.Set(rand_pos.x, rand_pos.y);
            p_BodyDef.linearDamping = 0.1;//Set body to non-rotatable
            p_body = world->CreateBody(&p_BodyDef);

            UserData* u = new UserData;
            u->type = PROP;
            u->data = this;
            p_body->SetUserData(u); //Store the Object to UserData

            b2CircleShape p_circleShape; 
            p_circleShape.m_p.Set(0, 0);
            p_circleShape.m_radius = p_radius; 
            b2FixtureDef p_FixtureDef;
            p_FixtureDef.shape = &p_circleShape;
            p_FixtureDef.density = 0;
            p_FixtureDef.filter.categoryBits = PROP; // Collision Filter
            p_FixtureDef.filter.maskBits = ME | PROP | WALL | BALL; // Collision Filter
            p_body->CreateFixture(&p_FixtureDef);

        }
        b2Body* body(){// return body in the class
            return p_body;    
        }
        _prop_type prop(){
            return prop_type;
        }
        void startContact() { p_numContacts++; } // ++ when the object attaches to something
        void endContact() { p_numContacts--; } // -- when the object dettaches to something
        void render() { //draw a face
        }
        void renderAtBodyPosition() {
        }
};


std::set<Ball*> ballsScheduledForRemoval;
std::set<Me*> mesScheduledForRemoval;
std::set<Prop*> propsScheduledForRemoval;
int a = 0;

class MyContactListener : public b2ContactListener// unuseable now
{
    void BeginContact(b2Contact* contact) {
        //check if fixture A was a ball
        UserData *UserData_A = static_cast<UserData*>(contact->GetFixtureA()->GetBody()->GetUserData());
        UserData *UserData_B = static_cast<UserData*>(contact->GetFixtureB()->GetBody()->GetUserData());
        _type type_A = UserData_A->type;
        _type type_B = UserData_B->type;
        if(type_A == EXP && type_B == BALL){
            static_cast<Ball*>(UserData_B->data)->startContact();
        }
        if(type_A == BALL && type_B == EXP){
            static_cast<Ball*>(UserData_A->data)->startContact();
        }
        if(type_A == BALL && type_B == WALL){
            ballsScheduledForRemoval.insert(static_cast<Ball*>(UserData_A->data));
        }
        if(type_A == WALL && type_B == BALL){
            ballsScheduledForRemoval.insert(static_cast<Ball*>(UserData_B->data));
        }
        if(type_A == ME && type_B == BALL){
            mesScheduledForRemoval.insert(static_cast<Me*>(UserData_A->data));
        }
        if(type_A == BALL && type_B == ME){
            mesScheduledForRemoval.insert(static_cast<Me*>(UserData_B->data));
        }
        if(type_A == PROP && type_B == ME){
            propsScheduledForRemoval.insert(static_cast<Prop*>(UserData_A->data));
            static_cast<Me*>(UserData_B->data)->eat_prop(static_cast<Prop*>(UserData_A->data)->prop());
        }
        if(type_A == ME && type_B == PROP){
            propsScheduledForRemoval.insert(static_cast<Prop*>(UserData_B->data));
            static_cast<Me*>(UserData_A->data)->eat_prop(static_cast<Prop*>(UserData_B->data)->prop());
        }
    }
    /*void EndContact(b2Contact* contact) {*/
        ////check if fixture A was a ball
        //void* bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
        //if ( bodyUserData )
            //static_cast<Ball*>( bodyUserData )->endContact();
        ////check if fixture B was a ball
        //bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData();
        //if ( bodyUserData )
            //static_cast<Ball*>( bodyUserData )->endContact();
    /*}*/
};

MyContactListener myContactListenerInstance;
std::set<b2Body*>particles;
class gg : public Test
{
    public:
    enum _moveState { // unuseable now
        MS_EXP,
        MS_STOP,
        MS_LEFT,
        MS_RIGHT,
        MS_UP,
        MS_DOWN,
    };
    enum _rot { // unuseable now // rotation
        STOP,
        LEFT,
        RIGHT,
    };

    _moveState moveState;
    _rot rot;
    b2Vec2 clickedPoint; //unuseable // mouse clicked point
    int timing;
    int xspeedup;
    int yspeedup;
    int stoptick;
    Me* me;
    b2Color red;
    b2Color green;
    b2Color blue;
    b2Vec2 def_pos;
    int expCD;
    int CD;
    float jizz_timing;
    std::vector<Ball*> balls;
    std::vector<Me*> mes;
    std::vector<Prop*> props;
    gg(){
        def_pos.x = 0;
        def_pos.y = 20;
        red.r = 1;
        red.g = 0;
        red.b = 0;
        green.r = 0;
        green.g = 1;
        green.b = 0;
        blue.r = 0;
        blue.g = 0;
        blue.b = 1;
        expCD = 0;
        jizz_timing = 0.001;
        CD = 150;
        me = new Me(m_world, 1, red, def_pos);
        moveState = MS_STOP;
        rot = STOP;
        m_world->SetContactListener(&myContactListenerInstance);
        m_world->SetGravity( b2Vec2(0,0) );

        b2BodyDef myBodyDef;
        b2PolygonShape mypolygonShape;
        b2FixtureDef myFixtureDef;
        myFixtureDef.shape = &mypolygonShape;
        myBodyDef.type = b2_staticBody;
        myBodyDef.position.Set(0, 0);
        b2Body* staticBody = m_world->CreateBody(&myBodyDef);
        UserData *u = new UserData;
        u->type = WALL;
        u->data = NULL;
        staticBody->SetUserData(u);
        myFixtureDef.filter.categoryBits = WALL;
        myFixtureDef.filter.maskBits = WALL | BALL | EXP | ME | PROP;

        mypolygonShape.SetAsBox( 20, 1, b2Vec2(0, 0), 0);//ground
        staticBody->CreateFixture(&myFixtureDef);
        mypolygonShape.SetAsBox( 20, 1, b2Vec2(0, 40), 0);//ceiling
        staticBody->CreateFixture(&myFixtureDef);
        mypolygonShape.SetAsBox( 1, 20, b2Vec2(-20, 20), 0);//left wall
        staticBody->CreateFixture(&myFixtureDef);
        mypolygonShape.SetAsBox( 1, 20, b2Vec2(20, 20), 0);//right wall
        staticBody->CreateFixture(&myFixtureDef);
        //mes.push_back();
    }
    void MouseDown(const b2Vec2& p){
        //store last mouse-down position
        clickedPoint = p;

        //do normal behaviour
        Test::MouseDown(p);
    }
    void Keyboard(unsigned char key){
       /* onKeyUp: function(key){*/
            //switch(key){
                //case 'c':
                    //break;
            //}
        //},
            //onKeyDown: function(key){
                //switch(key){
                    //case 'c':
                        //break;
                //}
            //}
        /*}*/
        switch(key){
                
            case 'b':
                if(me->prop() == AVATAR){
                    mes.push_back(new Me(m_world, 1, red, me->body()->GetPosition()));
                    me->eat_prop(NONE);
                }    
                break;
            case 'a': //move left
                moveState = MS_LEFT;
                break;
            case 'd': //move right
                moveState = MS_RIGHT;
                break;
            case 'w': //move up
                moveState = MS_UP;
                break;
            case 's': //move down
                moveState = MS_DOWN;
                break;
            case 'c' :
                moveState = MS_EXP;
                break;
            case 'j':
                rot = LEFT;
                break;
            case 'k':
                rot = RIGHT;
                break;
            default:
                //run default behaviour
                Test::Keyboard(key);
        }
    }
    void Step(Settings* settings){
        //run the default physics and rendering
        Test::Step(settings);
        if(me->prop() == JIZZ){
            CD = 1;
            expCD = 1;
            jizz_timing = 400;
            me->eat_prop(NONE);
        }
        timing++;
        if(expCD>0) expCD--;
        if(CD == 1){
            jizz_timing--;
        }
        if(jizz_timing == 0){
            CD = 150;
            jizz_timing = 0.01;
        }
        if(timing == 100000000){
            timing = 0;
        }
        if(timing%100 == 0){
            balls.push_back(new Ball(me->body(), m_world, 1, green));
        }
        if(timing%1000 == 0){
            props.push_back(new Prop(m_world, 1, blue, 1));
        }
        for(int i = 0; i < balls.size(); i++){
            balls[i]->renderAtBodyPosition();
            balls[i]->moveTowardMe();
        }
        b2Vec2 vel = me->body()->GetLinearVelocity();
        switch(moveState){
            case MS_EXP:
                if(expCD == 0){
                	for(int i = 0; i < 75; i++){
                        float angle = (i / (float)75) * 6.28;
                        b2Vec2 rayDir( sinf(angle), cosf(angle) );
                        b2BodyDef bd;
                        bd.type = b2_dynamicBody;
                        //bd.SetUserData(this->)
                        bd.fixedRotation = true; // rotation not necessary
                        bd.bullet = true; // prevent tunneling at high speed
                        bd.linearDamping = 8; // drag due to moving through air
                        bd.position = me->body()->GetWorldCenter(); // start at blast center
                        bd.linearVelocity = 200000 * rayDir;
                        b2Body* body = m_world->CreateBody( &bd );
                        UserData *u = new UserData;
                        u->type = EXP;
                        u->data = NULL;
                        body->SetUserData(u);
                        particles.insert(body);
                        b2CircleShape circleShape;
                        circleShape.m_radius = 0.06; // very small
                        b2FixtureDef fd;
                        fd.shape = &circleShape;
                        fd.density = 10; /// (float)575; // very high - shared across all particles
                        fd.friction = 0; // friction not necessary
                        fd.restitution = 0.3f; // high restitution to reflect off obstacles
                        fd.filter.categoryBits = EXP;
                        fd.filter.maskBits = BALL | WALL;
                        //fd.filter.categoryBits = 1;
                        //fd.filter.groupIndex = -1; // particles should not collide with each other
                        body->CreateFixture( &fd );
                        expCD = CD;
                    }
                }
                break;
            case MS_RIGHT: 
                if(vel.x <= 30) me->body()->ApplyForce( b2Vec2(4000,0), me->body()->GetWorldCenter() );
                break;
            case MS_LEFT:  
                if(vel.x >= -30)me->body()->ApplyForce( b2Vec2(-4000,0), me->body()->GetWorldCenter() );
                break;
            case MS_UP:    
                if(vel.y <= 30 ) me->body()->ApplyForce( b2Vec2(0,4000), me->body()->GetWorldCenter() );
                break;
            case MS_DOWN:  
                if(vel.y >=-30 )me->body()->ApplyForce( b2Vec2(0,-4000), me->body()->GetWorldCenter() );
                break;
        }
        moveState = MS_STOP;
        float bodyAngle = me->body()->GetAngle();
        float change = 1 * 0.0174532925; //allow 1 degree rotation per time step
        switch(rot){
            case LEFT: 
                me->body()->SetTransform(me->body()->GetPosition(), bodyAngle + 5*change);
                rot = STOP;
                break;
            case RIGHT:
                me->body()->SetTransform(me->body()->GetPosition(), bodyAngle - 5*change);
                rot = STOP;
                break;
        }
        std::set<b2Body*>::iterator it = particles.begin();
        std::set<b2Body*>::iterator end = particles.end();
        for(;it!=end;++it){
            if(abs((*it)->GetLinearVelocity().x) <= 0.01 && abs((*it)->GetLinearVelocity().y) <= 0.01){
                b2Body* gg = *it;
                particles.erase(it);
                m_world->DestroyBody(gg);
                //particles.clear();
            }   
        }
        std::set<Ball*>::iterator b_it = ballsScheduledForRemoval.begin();
        std::set<Ball*>::iterator b_end = ballsScheduledForRemoval.end();
        std::set<Me*>::iterator m_it = mesScheduledForRemoval.begin();
        std::set<Me*>::iterator m_end = mesScheduledForRemoval.end();
        std::set<Prop*>::iterator p_it = propsScheduledForRemoval.begin();
        std::set<Prop*>::iterator p_end = propsScheduledForRemoval.end();
               
        for (; b_it!=b_end; ++b_it) {
            Ball* dyingBall = *b_it;

            //delete ball... physics body is destroyed here
            //delete dyingBall;
            dyingBall->body()->GetWorld()->DestroyBody(dyingBall->body());
            //... and remove it from main list of balls
            std::vector<Ball*>::iterator b_it = std::find(balls.begin(), balls.end(), dyingBall);
            if ( b_it != balls.end() )
                balls.erase( b_it );
        }
        ballsScheduledForRemoval.clear();

        for (; m_it!=m_end; ++m_it) {
            Me* dyingMe = *m_it;

            //delete ball... physics body is destroyed here
            //delete dyingMe;
            dyingMe->body()->GetWorld()->DestroyBody(dyingMe->body());
            //... and remove it from main list of mes
            std::vector<Me*>::iterator m_it = std::find(mes.begin(), mes.end(), dyingMe);
            if ( m_it != mes.end() )
                mes.erase( m_it );
        }
        mesScheduledForRemoval.clear();

        for (; p_it!=p_end; ++p_it) {
            Prop* dyingProp = *p_it;

            //delete ball... physics body is destroyed here
            //delete dyingProp;
            dyingProp->body()->GetWorld()->DestroyBody(dyingProp->body());
            //... and remove it from main list of props
            std::vector<Prop*>::iterator p_it = std::find(props.begin(), props.end(), dyingProp);
            if ( p_it != props.end() )
                props.erase( p_it );
        }
        propsScheduledForRemoval.clear();
        //particles.clear();
        m_debugDraw.DrawString(5, m_textLine, "Current Score:%d ExplosionCD:%.1f PropEst:%.1f",timing/10,(float)expCD/60,(float)jizz_timing/60);
        m_textLine += 15;
        m_debugDraw.DrawString(5, m_textLine, "%d", CD);
        glPointSize(4);
        glBegin(GL_POINTS);
        glVertex2f(clickedPoint.x, clickedPoint.y);
        glEnd();
        
    }

    static Test* Create(){
        return new gg;
    }
};

