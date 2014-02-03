#version 330                                                                        
                                                                                    
layout (location = 0) in float Type;                                                
layout (location = 1) in vec3 Position;                                             
layout (location = 2) in vec3 Velocity;                                             
layout (location = 3) in float Age;                                                 
                                                                                    
out float Type0;                                                                    
out vec3 Position0;                                                                 
out vec3 Velocity0;                                                                 
out float Age0;                                                                     
                                                                                    
void main()                                                                         
{                                                                                   
    Type0 = Type;                                                                   
    Position0 = Position;                                                           
    Velocity0 = Velocity;                                                           
    Age0 = Age;                                                                     
}